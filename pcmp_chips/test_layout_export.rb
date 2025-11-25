# Simple KLayout export script
input_file = $input
output_file = $output

include RBA

app = Application.instance
mw = MainWindow.instance
view = mw.create_layout(0)

# Load layout
view.load_layout(input_file, 0)

# Zoom to fit
view.max_hier
view.zoom_fit

# Save image
view.save_image(output_file, 1920, 1080)

puts "Image saved to: #{output_file}"
