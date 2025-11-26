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

# Find the best top cell (chip-level)
# Strategy: Pick the largest top cell by bounding box area
top_cells = layout.top_cells
if top_cells.size > 0
  # If multiple top cells, pick the one with largest bounding box
  best_cell = top_cells[0]
  if top_cells.size > 1
    max_area = 0
    top_cells.each do |cell|
      bbox = cell.bbox
      if bbox.empty?
        area = 0
      else
        area = bbox.width * bbox.height
      end
      if area > max_area
        max_area = area
        best_cell = cell
      end
    end
    puts "Multiple top cells found (#{top_cells.size}), selected largest: #{best_cell.name || best_cell.cell_index}"
  end
  cell_view.cell_name = best_cell.name || best_cell.cell_index.to_s
end

# Load layer properties from session if available
# This applies our custom layer mapping for newer chips
if session_file && File.exist?(session_file)
  begin
    lv.load_layer_props(session_file)
    puts "Loaded layer properties from: #{session_file}"
  rescue => e
    puts "Warning: Could not load session file: #{e.message}"
  end
end

# Add missing layers - this adds layer entries for any layers in the layout
# that aren't already in the layer properties (e.g., older CIF files with LCP, LCM, etc.)
# This is equivalent to using "Add Other Layers" from the context menu in the layers panel
begin
  lv.add_missing_layers
  puts "Added missing layers to layer properties"
rescue => e
  puts "Warning: Could not add missing layers: #{e.message}"
  # Fallback: try init_layer_props if add_missing_layers doesn't exist
  begin
    lv.init_layer_props
    puts "Initialized layer properties (fallback)"
  rescue => e2
    puts "Warning: Could not initialize layer properties: #{e2.message}"
  end
end

# Zoom to fit
lv.max_hier
lv.zoom_fit

# Save image
lv.save_image(output_file, width, height)

puts "Created: #{output_file}"
