import sys
import os
import glob
import platform
from pathlib import Path

def compile_shaders(executable, base_args, lang_base_args, permutation_args, includes_args, shaders_dir, shader_ext, target_profile_fn, output_path):

	for shader_file in [f for f in shaders_dir.rglob("*." + shader_ext) if f.is_file()]:
		shader_file_name = shader_file.stem

		target_profile = target_profile_fn(shader_file_name)

		command = executable+" "+base_args+" "+lang_base_args+" "+permutation_args+" "+"-name="+shader_file_name+"_16bit"+" -DFFXM_HALF=1 "
		command += target_profile+" "+includes_args+" "+"-output="+output_path+" "+str(shader_file)
		print(f"command is {command}")
		os.system(command)

def main():
	use_glsl = "--glsl" in sys.argv

	script_folder = str(Path(os.path.abspath(os.path.dirname(__file__))))

	if platform.system() == "Windows":
		ffxm_sc_executable = script_folder+"/bin/FidelityFX_SC.exe"
	else:
		ffxm_sc_executable = script_folder+"/bin/FidelityFX_SC"

	fsr2_base_args = "-reflection -deps=gcc -DFFXM_GPU=1"

	fsr2_permutation_args = "-DFFXM_FSR2_OPTION_HDR_COLOR_INPUT={0,1}"
	fsr2_permutation_args += " -DFFXM_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS={0,1}"
	fsr2_permutation_args += " -DFFXM_FSR2_OPTION_JITTERED_MOTION_VECTORS={0,1}"
	fsr2_permutation_args += " -DFFXM_FSR2_OPTION_INVERTED_DEPTH={0,1}"
	fsr2_permutation_args += " -DFFXM_FSR2_OPTION_APPLY_SHARPENING={0,1}"
	fsr2_permutation_args += " -DFFXM_FSR2_OPTION_SHADER_OPT_BALANCED={0,1}"
	fsr2_permutation_args += " -DFFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE={0,1}"
	fsr2_permutation_args += " -DFFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE={0,1}"

	fsr2_include_args = "-I"+script_folder+"/../include/gpu/"+" -I"+script_folder+"/../include/gpu/fsr2"

	output_path = script_folder+"/../src/backends/shared/blob_accessors/prebuilt_shaders"

	if use_glsl:
		fsr2_lang_base_args = "-compiler=glslang -e main --target-env vulkan1.1 -Os -DFFXM_GLSL=1"
		shaders_dir = Path(f"{script_folder}/../src/backends/vk/shaders/fsr2/glsl")
		shader_ext = "glsl"

		def target_profile_fn(name):
			if name.endswith("_vs"):
				return "-S vert"
			elif name.endswith("_fs"):
				return "-S frag"
			else:
				return "-S comp"
	else:
		fsr2_lang_base_args = "-E main -Wno-for-redefinition -Wno-ambig-lit-shift -DFFXM_HLSL=1 -DFFXM_HLSL_6_2=1 -spirv -fspv-target-env=vulkan1.1spirv1.4 -Wall -Wextra -Wno-conversion -Wundef -Wno-unused-const-variable -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unknown-pragmas -Wuninitialized -HV 2021"
		shaders_dir = Path(f"{script_folder}/../src/backends/vk/shaders/fsr2/hlsl")
		shader_ext = "hlsl"

		def target_profile_fn(name):
			if name.endswith("_vs"):
				return "-enable-16bit-types -T vs_6_7"
			elif name.endswith("_fs"):
				return "-enable-16bit-types -T ps_6_7"
			else:
				return "-enable-16bit-types -T cs_6_7"

	for f in glob.glob(output_path+"/*"):
		os.remove(f)

	compile_shaders(ffxm_sc_executable, fsr2_base_args, fsr2_lang_base_args, fsr2_permutation_args, fsr2_include_args, shaders_dir, shader_ext, target_profile_fn, output_path)

if __name__ == "__main__":
    main()
