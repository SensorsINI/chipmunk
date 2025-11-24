# KLayout CIF to PNG export script for headless mode
input_file = $input
output_file = $output
session_file = $session
width = ($width || 1920).to_i
height = ($height || 1080).to_i

include RBA

# Load layout
layout = Layout.new
layout.read(input_file)

# Create layout view
lv = LayoutView.new(true)  # true = editable

# Load layout into view
cell_view_index = lv.create_layout(true)
cell_view = lv.cellview(cell_view_index)
cell_view.layout.assign(layout)

# Set cell name - handle multiple top cells
top_cells = layout.top_cells
if top_cells.size > 0
  cell_view.cell_name = top_cells[0].name
end

# Load layer properties from session if available
if session_file && File.exist?(session_file)
  begin
    lv.load_layer_props(session_file)
  rescue => e
    puts "Warning: Could not load session file: #{e.message}"
  end
end

# Zoom to fit
lv.max_hier
lv.zoom_fit

# Save image
lv.save_image(output_file, width, height)

puts "Created: #{output_file}"
