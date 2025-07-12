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

Write-Host "earlyZPass shader file: $earlyZPassFile"
Write-Host "earlyZPass root signature output: $earlyZPassRootSignatureOutput"
Write-Host "earlyZPass vertex shader output: $earlyZPassVertexShaderOutput"

# Compile root signature
& $dxc /T rootsig_1_1 $earlyZPassFile /E "EarlyZPassRootsignature" /Fo $earlyZPassRootSignatureOutput /nologo
# Compile vertex shader
& $dxc /T vs_6_6 $earlyZPassFile /E"VertexShaderMain" /Fo $earlyZPassVertexShaderOutput /nologo

Write-Host "Shader Build Done!"
Write-Host "************************"
Write-Host "************************"

###### debug arg /Zi /Fd $path