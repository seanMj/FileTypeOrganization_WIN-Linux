//by. Sean Jesky i hope this helps someone out there. Please use in a responsible way.. 
/*
 * FILE('s): main.cpp
 * DATE&TIME: feb 15th 2023
 * CREATED_BY: Sean Jesky ()
 * CREATED_FOR: the dream
 * WHAT_IT_DOES:  This data is saved in users requested location. Segregation and organization of file extension type.

 */

#include <filesystem>
namespace fs = std::filesystem;
#include <exception>
#include <vector>
using std::vector;
#include <iostream>
#include <string>
using std::string;
#include <algorithm>
#include  <fstream>
using std::ofstream;

/// @brief collect all files from the system to use as the data set for file organization
/// @param fs::path directory (const)
/// @return vector of fs::path
const inline vector<fs::path> collect_relivant_files(const fs::path directory)
{

	//save file names to files;
	vector<fs::path>* p_vfiles = new std::vector<fs::path>;
	p_vfiles->reserve(50'000);
	try {
		for (const fs::directory_entry& dir : fs::recursive_directory_iterator{ directory })
		{
			if (!fs::is_empty(dir)) {
				const fs::path& file_path = dir;
				const fs::path* file_path_p = &file_path;

				if (!fs::is_directory(*file_path_p) && fs::file_size(*file_path_p) > 0)
				{
					if (file_path_p->has_extension())
					{
						p_vfiles->push_back(*file_path_p);
					}
				}
			}
		}
	}
	catch (std::filesystem::filesystem_error& err) {
		std::cout << "Filesys error" << "\n" << err.what() << std::endl;
	}

	std::sort(p_vfiles->begin(), p_vfiles->end());

	std::vector<fs::path> files = *p_vfiles;
	delete p_vfiles;
	return files;
}



/// @brief get extensions based on file extensions within lookup directory and push back one of each to vector to exclude duplicates:
/// @param *collected_data (const)
/// @return vector of fs::path
const inline vector<fs::path> get_extensions(const vector<fs::path>* collected_data)
{
	std::vector<std::string>* p_types = new std::vector<std::string>;
	//vector<string>* p_types = &extension_types;
	//to increasse the speed of the code, reserve the amount of space that will be needed by the code
	p_types->reserve(collected_data->size());
	//go through the collected data(every file and directory)
	for (const auto& current_filename : (*(collected_data)))
	{
		//if the file has a extension, this is the data we need, continue
		if (current_filename.has_extension())
		{
			//save the extensions of all the files [this can be combined with the steps below, or moved into its own function]
			std::string path_str = current_filename.extension().string();
			p_types->push_back(path_str);
		}
	}
	//create a vector to save the valid file extension data within
	vector<fs::path>* types_no_duplicates = new std::vector<fs::path>;
	//vector<fs::path>* p_types_no_duplicates = &types_no_duplicates;
	//sort the data by alpha. order.
	std::sort(p_types->begin(), p_types->end());
	//go through the sorted vector one by one:
	for (decltype(p_types->size()) i = 0; i != p_types->size(); ++i)
	{
		//if the value from the vector doesn't repeat itself..  fix ERROR
		if ((*(p_types))[i] != (*(p_types))[i - 1])
		{
			//then place it in the new vector, this is the list that will create the dir.list to copy to
			types_no_duplicates->push_back((*(p_types))[i]);
			//print the extensions that will become the directories to copy to
			std::cout << "[*]\t" << (*(p_types))[i] << '\t' << '\n';
		}
	}
	delete p_types;
	std::vector<fs::path> type_nd = *types_no_duplicates;
	delete types_no_duplicates;
	//send the valid file extension type data to vector for creating the directories
	return type_nd;
}


/// @brief create a directory based off the extension type of files provided from a vector of pointers to fs::path extensions
/// @param fs::path base_save_location (const)
/// @param vector<fs::path> *extensions (const)
inline void create_directory_from_extension_type(const fs::path base_save_location, const vector<fs::path>* extensions)
{
	//vector<fs::path> save_location;
	for (auto& i : (*(extensions)))	//go through extension types presented from prev. function.
	{
		fs::create_directory((base_save_location.string()) + i.string().erase(0, 1));		//create dir based on extensions
	}
}


/// @brief copy files from the computer's filesystem into a directory for the user
/// @param fs::path dest (const)
/// @param vector<fs::path>* files_to_copy (const)
/// @param vector<fs::path>* extensions (const)
void copy_relevent_files(const fs::path dest, const vector<fs::path>* files_to_copy, const vector<fs::path>* extensions)
{
	//call function to create the directorys needed to copy to
	create_directory_from_extension_type(dest, extensions);
	for (const auto& ext : *extensions)
	{
		int i = 0;
		int* p_i = &i;
		for (const auto& files : (*(files_to_copy)))
		{
			if (files.has_extension())
			{
				if (files.extension() == ext)
				{
					//std::cout << "copying: " << files << '\t' << files.extension() << '\n';
					const fs::path core_path = dest.string() + ext.string().erase(0, 1);
					try
					{
						//copy files to new location that are based on its extension
						fs::copy(files, (core_path.string()), fs::copy_options::recursive | fs::copy_options::update_existing);
					}
					//if an error occurs, print error in log file for later lookup if needed by user.
					catch (std::runtime_error& err)
					{
						ofstream logs_(dest.string() + "error.txt", std::ios::app);
						logs_ << err.what() << "\n" << dest.string() + ext.string() << '\n';
					}
					//create the filename destination and save location to be used by the functions below:
					const fs::path save_this = core_path.string() + "\\" + files.filename().string();
					const fs::path save_at = save_this.string() + "_" + std::to_string(++(*(p_i))) + "_" + std::to_string(time(NULL)) + files.extension().string();
					//rename all the files in their new location....after they have been copied, this should be rewritten with Multi-threads.
					if (fs::exists(save_this))
					{
						fs::rename(save_this, save_at);
					}
				}
			}    //comp = false;
		}
	}
}


/// @brief (FTO Version 1.0) This program goes through a filesystem and gets file extensions, creates directories from those extensions, and copies files to the corresponding directory extension name 
/// @param int argv 
/// @param char *argc 
/// @return 
int main(int argv, char* argc[])
{
	//used dynamic memory alloc. because i wanted this to be as effecent as possible...
	// turns out dynamic mem. is not so important when using the standard library filesystem
	//TODO: add input checks for the user so we dont go all crazy with malformed input just in case.
	const std::string copy_from = argc[1];
	const fs::path path_to_copy_from = copy_from;

	//const fs::path path_to_copy_from = "";
	vector<fs::path>* p_files = new vector<fs::path>;
	//vector<fs::path>* p_files = &files;
	*p_files = collect_relivant_files(path_to_copy_from);

	vector<fs::path>* p_extn = new vector<fs::path>;
	//vector<fs::path>* p_extn = &extn;
	*p_extn = get_extensions(p_files);

	const std::string save_location = argc[2];
	const fs::path base_save_location = save_location;

	std::cout << "\n[+] There are:\t" << p_files->size() << "\tFiles To Be Copied...\n";
	std::cout << "[+] There are:\t" << p_extn->size() << "\tExtensions Directories To Be Created...\n";

	if (p_files->size() >= 10'000) {
		std::cout << "\t\tPlease Be Patient...\n\n";
	}
	//#pragma omp parallel
	copy_relevent_files(base_save_location, p_files, p_extn);
	//#pragma omp barrier

	delete p_files;
	delete p_extn;

}
