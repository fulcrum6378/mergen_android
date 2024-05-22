clspv cpp/vis/edge_detection.cl `
    --cl-std=CL3.0 `
    --spv-version=1.0 `
    -o android/assets/edge_detection_with_reflect.spv

#--spv-version=<?>
#             =1.0  - Vulkan 1.0
#             =1.3  - Vulkan 1.1 (experimental)
#             =1.4  - Vulkan 1.1 (experimental)
#             =1.5  - Vulkan 1.2 (experimental)
#             =1.6  - Vulkan 1.3 (experimental)

spirv-opt `
    --remove-duplicates `
    --strip-reflect `
    --trim-capabilities `
    android/assets/edge_detection_with_reflect.spv `
    -o android/assets/edge_detection.spv

Remove-Item  android/assets/edge_detection_with_reflect.spv

