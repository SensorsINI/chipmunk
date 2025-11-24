# KLayout Ruby script to export layout as PNG with session colors
# Usage: klayout -zz -r generate_layout_images.rb -rd input=<file> -rd output=<png> -rd session=<lys>

include RBA

input_file = $input
output_file = $output
session_file = $session
width = ($width || 1920).to_i
height = ($height || 1080).to_i

begin
  # Load layout
  app = Application.instance
  mw = MainWindow.instance
  
  # Load session if provided (contains layer colors/patterns)
  if session_file && File.exist?(session_file)
    mw.restore_session(session_file)
  end
  
  # Create or get view
  view = mw.current_view
  if !view
    view = mw.create_layout(1)
  end
  
  # Load the layout file
  view.load_layout(input_file, 0)
  
  # Zoom to fit all content
  view.max_hier
  view.zoom_fit
  
  # Export as PNG
  view.save_image(output_file, width, height)
  
  puts "Successfully created: #{output_file}"
  app.exit(0)
  
rescue => e
  puts "Error: #{e.message}"
  puts e.backtrace.join("\n")
  app.exit(1)
end
