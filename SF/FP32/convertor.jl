using SoftPosit

# Include the weights file
include("weights.jl")

function convert_weights_to_posit8()
    try
        # Convert fc1_weight to Posit8 bitstrings
        fc1_posit8 = similar(fc1_weight, String)
        for i in 1:size(fc1_weight, 1)
            for j in 1:size(fc1_weight, 2)
                fc1_posit8[i,j] = bitstring(Posit8(fc1_weight[i,j]))
            end
        end
        
        # Convert fc2_weight to Posit8 bitstrings
        fc2_posit8 = similar(fc2_weight, String)
        for i in 1:size(fc2_weight, 1)
            for j in 1:size(fc2_weight, 2)
                fc2_posit8[i,j] = bitstring(Posit8(fc2_weight[i,j]))
            end
        end
        
        # Save to a new .cpp file with posit objects
        open("weights_posit8.cpp", "w") do f
            write(f, "// Converted weights to Posit8 objects in C++ format\n")
            write(f, "#include <array>\n")
            write(f, "#include \"posit.h\"\n\n")
            
            # fc1_posit8 (784x256)
            write(f, "std::array<std::array<posit, 256>, 784> fc1_posit8 = {\n")
            for i in 1:size(fc1_posit8, 1)
                write(f, "    {")
                for j in 1:size(fc1_posit8, 2)
                    bits = replace(fc1_posit8[i,j], " " => "")  # Remove spaces
                    write(f, "posit(0b" * bits * ", 8, 2)")
                    write(f, j < size(fc1_posit8, 2) ? ", " : "")
                end
                write(f, "},\n")
            end
            write(f, "};\n\n")
            
            # fc2_posit8 (256x10)
            write(f, "std::array<std::array<posit, 10>, 256> fc2_posit8 = {\n")
            for i in 1:size(fc2_posit8, 1)
                write(f, "    {")
                for j in 1:size(fc2_posit8, 2)
                    bits = replace(fc2_posit8[i,j], " " => "")
                    write(f, "posit(0b" * bits * ", 8, 2)")
                    write(f, j < size(fc2_posit8, 2) ? ", " : "")
                end
                write(f, "},\n")
            end
            write(f, "};\n")
        end
        println("Conversion complete! Saved to weights_posit8.cpp")
        
    catch e
        println("Error occurred: ", e)
    end
end

# Run the conversion
convert_weights_to_posit8()