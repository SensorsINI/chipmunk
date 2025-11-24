# KLayout headless export script
input_file = $input
output_file = $output
width = ($width || 1920).to_i
height = ($height || 1080).to_i

include RBA

# Load layout
layout = Layout.new
layout.read(input_file)

# Create a layout view for rendering
lv = LayoutView.new

# Set layout
lv.add_layout(layout, true)

# Select all cells
lv.select_all

# Zoom fit
lv.zoom_fit

# Save image
lv.save_image(output_file, width, height)

puts "Saved: #{output_file}"
