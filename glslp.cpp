#include <iostream>
#include <filesystem>
#include <algorithm>

void showHelp()
{
	std::cout << "Example : glslp.exe -dir <dir>" << std::endl;
	std::cout << "-dir <dir> : The folder in which your .glsl files are stored" << std::endl;
	std::cout << "-help      : Show this information" << std::endl;
}

void convertShaders(char* root)
{
	auto path = std::tr2::sys::initial_path<std::tr2::sys::path>() / std::tr2::sys::path(root);

	// if our path is the full path, then just use it
	if (root[1] == ':')
	{
		path = std::tr2::sys::path(root);
	}

	std::string hpath(path);
	hpath.append("\\shaders.h");

	std::ofstream hfile(hpath);
	hfile << "#pragma once" << std::endl << std::endl;
	hfile << "// WARNING - GENERATED FILE, ALL MODIFICATIONS WILL BE OVERWRITTEN!" << std::endl << std::endl;
	hfile << "namespace shader" << std::endl;
	hfile << "{" << std::endl;

	std::string cpath(path);
	cpath.append("\\shaders.cpp");

	std::ofstream cfile(cpath);
	cfile << "#include \"shaders.h\"" << std::endl << std::endl;
	cfile << "// WARNING - GENERATED FILE, ALL MODIFICATIONS WILL BE OVERWRITTEN!" << std::endl << std::endl;
	cfile << "namespace shader" << std::endl;
	cfile << "{" << std::endl;

	int numShadersConvereted = 0;

	for (auto it = std::tr2::sys::recursive_directory_iterator(path); it != std::tr2::sys::recursive_directory_iterator(); ++it)
	{
		const auto& file = it->path();

		std::string fstr = file.filename();

		if (std::tr2::sys::is_regular_file(file))
		{

			if (fstr.find(".glsl") != std::string::npos)
			{
				//std::cout << "Checking file : " << fstr << std::endl;
				hfile << " // shader file " << file.directory_string() << std::endl;
				cfile << " // shader file " << file.directory_string() << std::endl;

				size_t p = fstr.find_last_of(".");
				if (p == std::string::npos)
				{
					std::cout << "Something wen't really wrong!" << std::endl;
				}
				fstr[p] = '_';

				hfile << " extern const char* " << fstr << ";" << std::endl;
				cfile << " const char* " << fstr << " = " << std::endl;

				numShadersConvereted++;

				// open the current file
				std::ifstream ofile(file.directory_string());

				std::string str;

				// Just use the same code for looking for comments as I had in glelp

				// this is set when we have a line which has a non closed /* in it
				// then we ignore each line/partial line until we find a */
				bool insideMultilineComment = false;

				// read it, line by line
				while (std::getline(ofile, str))
				{
					// if we are inside a multiline comment
					if (insideMultilineComment)
					{
						std::size_t epos = str.find("*/");
						// When we find the end of the comment, we cut of the part before
						// the comment and check the remaing parts further down
						if (epos != std::string::npos)
						{
							str = str.substr(epos + 2, std::string::npos);
							insideMultilineComment = false;
						}
						// this line should be ignored
						else
						{
							continue;
						}
					}

					// check if there is a comment on this line
					std::size_t pos = str.find("//");
					if (pos != std::string::npos)
					{
						str = str.substr(0, pos);
					}

					// might be multiple comments on the same row
					// like foo /* bar */ foo /* bar */ foo /* bar
					// in which we should keep the foo, and ignore the bar
					while (true)
					{
						std::size_t pos = str.find("/*");
						if (pos != std::string::npos)
						{
							// see if the comment ends on this line
							std::size_t epos = str.find("*/");
							// if it does, cut out the comment
							if (epos != std::string::npos)
							{
								str = str.erase(pos, (epos - pos) + 2);
							}
							// nope, then we are inside a multiline comment
							// and should just start ignoring things,
							// but first we need to check whats before the comment
							else
							{
								insideMultilineComment = true;
								str = str.substr(0, pos);

								// break the while loop
								break;
							}
						}
						else
						{
							// break the while loop
							break;
						}
					}

					// if it's only white spaces.. then we don't need to care
					if (!std::all_of(str.begin(), str.end(), isspace))
					{
						// remove all tabs
						str.erase(std::remove(str.begin(), str.end(), '\t'), str.end());

						// trim the white spaces
						size_t first = str.find_first_not_of(' ');
						if (std::string::npos != first)
						{
							size_t last = str.find_last_not_of(' ');
							str = str.substr(first, (last - first + 1));
						}

						// if the line contains a # it needs to be handled differently
						if (str.find('#') == std::string::npos)
						{
							// if the lines ends with anything else we need to add an additional space	
							size_t p = str.find_last_not_of("[;{}\\)]");
							if ((p == std::string::npos) || (p != (str.size() - 1)))
							{
								cfile << " \"" << str << "\"" << std::endl;
							}
							else
							{
								cfile << " \"" << str << " \"" << std::endl;
							}
						}
						else
						{
							cfile << " \"\\n" << str << "\\n\"" << std::endl;
						}

					}
				}

				//if (foundInFile)
				//	std::cout << "found gl call in " << file.filename() << std::endl;

				// close the current file
				ofile.close();

				cfile << " \"\";" << std::endl << std::endl;
			}
		}

	}

	hfile << "}" << std::endl << std::endl;
	hfile.close();

	cfile << "}" << std::endl << std::endl;
	cfile.close();

	std::cout << numShadersConvereted << " shaders converted" << std::endl << std::endl;
}

int main(int argc, char* argv[])
{
	std::cout << "GLSLP 0.1 - OpenGL Shader Helper" << std::endl;
	std::cout << "2016 (c) Anders Malm - Ekoli / Odious ^ S/N ^ Censor" << std::endl << std::endl;

	if (argc == 1)
	{
		std::cout << "ERROR - You need to provide arguments" << std::endl;
		showHelp();
	}

	for (int arg = 1; arg < argc; arg++)
	{
		std::string str(argv[arg]);
		if ((0 == str.compare("-dir")) && ((arg + 1) < argc))
		{
			convertShaders(argv[++arg]);
			return 0;
		}
	}

	return 0;
}