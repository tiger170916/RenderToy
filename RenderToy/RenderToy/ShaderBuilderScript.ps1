param(
	[string]$fxc,
	[string]$shaderDir,
	[string]$outputDir
)

Write-Host "`nBuilding shaders..."
Write-Host "FXC path : $fxc"
Write-Host "shader source path : $shaderDir"
Write-Host "shader binary output path : $outputDir"

###### EarlyZPass #######
$earlyZPassFile = $shaderDir + "EarlyZPass.hlsl"
$earlyZPassRootSignatureOutput = $outputDir + "early_z_pass_root_signature.cso"
Write-Host "earlyZPass shader file: $earlyZPassFile"
Write-Host "earlyZPass root signature output: $earlyZPassRootSignatureOutput"

# Compile root signature
& $fxc /T rootsig_1_1 $earlyZPassFile /E "EarlyZPassRootsignature" /Fo $earlyZPassRootSignatureOutput /nologo


Write-Host "Shader Build Done!"
Write-Host "************************"
Write-Host "************************"
