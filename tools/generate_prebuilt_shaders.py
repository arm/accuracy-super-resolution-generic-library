import sys
import os
import glob
from pathlib import Path

def compile_shaders(executable, base_args, hlsl_base_args, permutation_args, includes_args, shaders_dir, output_path):

	for shader_file in [f for f in shaders_dir.rglob("*.hlsl") if f.is_file()]:
		shader_file_name = shader_file.stem

		if (shader_file_name.endswith("_vs")):
			target_profile = "-T vs_6_7"
		elif (shader_file_name.endswith("_fs")):
			target_profile = "-T ps_6_7"
		else:
			target_profile = "-T cs_6_7"

		command = executable+" "+base_args+" "+hlsl_base_args+" "+permutation_args+" "+"-name="+shader_file_name+"_16bit"+" -DFFXM_HALF=1 "
		command += "-enable-16bit-types "+target_profile+" "+includes_args+" "+"-output="+output_path+" "+str(shader_file)
		print(f"command is {command}")
		os.system(command)

def main():
	script_folder = str(Path(os.path.abspath(os.path.dirname(__file__))))

	ffxm_sc_executable = script_folder+"/bin/FidelityFX_SC.exe"

	fsr2_base_args = "-reflection -deps=gcc -DFFXM_GPU=1"

	fsr2_hlsl_base_args = "-E main -Wno-for-redefinition -Wno-ambig-lit-shift -DFFXM_HLSL=1 -DFFXM_HLSL_6_2=1 -spirv -fspv-target-env=vulkan1.1spirv1.4 -Wall -Wextra -Wno-conversion -Wundef -Wno-unused-const-variable -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unknown-pragmas -Wuninitialized -HV 2021"

	fsr2_permutation_args = "-DFFXM_FSR2_OPTION_HDR_COLOR_INPUT={0,1}"
	fsr2_permutation_args += " -DFFXM_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS={0,1}"
	fsr2_permutation_args += " -DFFXM_FSR2_OPTION_JITTERED_MOTION_VECTORS={0,1}"
	fsr2_permutation_args += " -DFFXM_FSR2_OPTION_INVERTED_DEPTH={0,1}"
	fsr2_permutation_args += " -DFFXM_FSR2_OPTION_APPLY_SHARPENING={0,1}"
	fsr2_permutation_args += " -DFFXM_FSR2_OPTION_SHADER_OPT_BALANCED={0,1}"
	fsr2_permutation_args += " -DFFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE={0,1}"

	fsr2_include_args = "-I "+script_folder+"/../include/gpu/"+" -I "+script_folder+"/../include/gpu/fsr2"

	shaders_dir = Path(f"{script_folder}/../src/backends/vk/shaders/fsr2/hlsl")

	output_path = script_folder+"/../src/backends/shared/blob_accessors/prebuilt_shaders"

	for f in glob.glob(output_path+"/*"):
		os.remove(f)

	compile_shaders(ffxm_sc_executable, fsr2_base_args, fsr2_hlsl_base_args, fsr2_permutation_args, fsr2_include_args, shaders_dir, output_path)

if __name__ == "__main__":
    main()
