param(
	[string]$dxc,
	[string]$shaderDir,
	[string]$outputDir
)

Write-Host "`nBuilding shaders..."
Write-Host "dxc path : $dxc"
Write-Host "shader source path : $shaderDir"
Write-Host "shader binary output path : $outputDir"

###### EarlyZPass #######
$earlyZPassFile = $shaderDir + "EarlyZPass.hlsl"
$earlyZPassRootSignatureOutput = $outputDir + "early_z_pass_root_signature.cso"
$earlyZPassVertexShaderOutput = $outputDir + "early_z_pass_vertex_shader.cso"

Write-Host "EarlyZPass shader file: $earlyZPassFile"
Write-Host "EarlyZPass root signature output: $earlyZPassRootSignatureOutput"
Write-Host "EarlyZPass vertex shader output: $earlyZPassVertexShaderOutput"

# Compile root signature
& $dxc /T rootsig_1_1 $earlyZPassFile /E "EarlyZPassRootsignature" /Fo $earlyZPassRootSignatureOutput /nologo
# Compile vertex shader
& $dxc /T vs_6_6 $earlyZPassFile /E"VertexShaderMain" /Fo $earlyZPassVertexShaderOutput /nologo

##### GeometryPass #####
$geometryPassFile = $shaderDir + "GeometryPass.hlsl"
$geometryPassRootSignatureOutput = $outputDir + "geometry_pass_root_signature.cso"
$geometryPassVertexShaderOutput = $outputDir + "geometry_pass_vertex_shader.cso"
$geometryPassPixelShaderOutput = $outputDir + "geometry_pass_pixel_shader.cso"
$geometryPassPixelShaderPdbOutput = $outputDir + "geometry_pass_pixel_shader.pdb"

# Compile root signature
& $dxc /T rootsig_1_1 $geometryPassFile /E "GeometryPassRootsignature" /Fo $geometryPassRootSignatureOutput /nologo
# Compile vertex shader
& $dxc /T vs_6_6 $geometryPassFile /E"VertexShaderMain" /Fo $geometryPassVertexShaderOutput /nologo
# Compile pixel shader
& $dxc /T ps_6_6 $geometryPassFile /E"PixelShaderMain" /Fo $geometryPassPixelShaderOutput /Zi /Fd $geometryPassPixelShaderPdbOutput /nologo

##### LightingPass #####
$lightingPassFile = $shaderDir + "LightingPass.hlsl"
$lightingPassRootSignatureOutput = $outputDir + "lighting_pass_root_signature.cso"
$lightingPassVertexShaderOutput = $outputDir + "lighting_pass_vertex_shader.cso"
$lightingPassVertexShaderPdbOutput = $outputDir + "lighting_pass_vertex_shader.pdb"
$lightingPassPixelShaderOutput = $outputDir + "lighting_pass_pixel_shader.cso"
$lightingPassPixelShaderPdbOutput = $outputDir + "lighting_pass_pixel_shader.pdb"

Write-Host "LightingPass shader file: $lightingPassFile"
Write-Host "LightingPass root signature output: $lightingPassRootSignatureOutput"
Write-Host "LightingPass vertex shader output: $lightingPassVertexShaderOutput"
Write-Host "LightingPass pixel shader output: $lightingPassPixelShaderOutput"

# Compile root signature
& $dxc /T rootsig_1_1 $lightingPassFile /E "LightingPassRootsignature" /Fo $lightingPassRootSignatureOutput /nologo
# Compile vertex shader
& $dxc /T vs_6_6 $lightingPassFile /E"VertexShaderMain" /Fo $lightingPassVertexShaderOutput /Zi /Fd $lightingPassVertexShaderPdbOutput /nologo
# Compile pixel shader
& $dxc /T ps_6_6 $lightingPassFile /E"PixelShaderMain" /Fo $lightingPassPixelShaderOutput /Zi /Fd $lightingPassPixelShaderPdbOutput /nologo

Write-Host "Shader Build Done!"
Write-Host "************************"
Write-Host "************************"

###### debug arg /Zi /Fd $pdb_path