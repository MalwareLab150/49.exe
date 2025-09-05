from PIL import Image

VGA_COLORS = [
    (0x00, 0x00, 0x00),  
    (0x00, 0x00, 0xAA),  
    (0x00, 0xAA, 0x00),  
    (0x00, 0xAA, 0xAA),  
    (0xAA, 0x00, 0x00),  
    (0xAA, 0x00, 0xAA),  
    (0xAA, 0x55, 0x00),  
    (0xAA, 0xAA, 0xAA),  
    (0x55, 0x55, 0x55),  
    (0x55, 0x55, 0xFF),  
    (0x55, 0xFF, 0x55),  
    (0x55, 0xFF, 0xFF),  
    (0xFF, 0x55, 0x55),  
    (0xFF, 0x55, 0xFF),  
    (0xFF, 0xFF, 0x55),  
    (0xFF, 0xFF, 0xFF),  
]

def closest_vga_color(rgb):
    r, g, b = rgb
    best_idx = 0
    best_dist = float('inf')
    for i, (vr, vg, vb) in enumerate(VGA_COLORS):
        dist = (r - vr)**2 + (g - vg)**2 + (b - vb)**2
        if dist < best_dist:
            best_idx = i
            best_dist = dist
    return best_idx

def image_to_c_array(image_path, width=80, height=25):
    img = Image.open(image_path).convert('RGB')
    img = img.resize((width, height))

    lines = [
        "#include <stdint.h>",
        "",
        f"const uint8_t image_chars[{width * height}] = {{"
    ]
    attr_lines = [
        f"const uint8_t image_colors[{width * height}] = {{"
    ]

    for y in range(height):
        row_chars = []
        row_attrs = []
        for x in range(width):
            r, g, b = img.getpixel((x, y))
            brightness = int((r + g + b) / 3)
            vga_color = closest_vga_color((r, g, b))

            if brightness > 220:
                char = 0xDB  
            elif brightness > 180:
                char = 0xB2  
            elif brightness > 120:
                char = 0xB1  
            elif brightness > 60:
                char = 0xB0  
            else:
                char = 0x20 

            row_chars.append(f"0x{char:02X}")
            attr_byte = (0 << 4) | vga_color  
            row_attrs.append(f"0x{attr_byte:02X}")
        lines.append("    " + ", ".join(row_chars) + ",")
        attr_lines.append("    " + ", ".join(row_attrs) + ",")

    lines.append("};")
    attr_lines.append("};")

    return "\n".join(lines + [""] + attr_lines)

if __name__ == "__main__":
    c_code = image_to_c_array("TEST.png")
    with open("image_data.h", "w") as f:
        f.write(c_code)
