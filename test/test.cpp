
#include <iostream>
#include <fusecpp.hpp>

void print_path(fusecpp::fusecpp_entry const& entry)
{
    std::cout << __func__ << "(): " << entry.path().string() << std::endl;
}

void do_ls_entry(fusecpp::directory const& dir)
{
    for (auto& ptr : dir.entries())
        std::cout << __func__ << "(): "
            << (ptr->is_file() ? "FILE" : "DIR")
            << ": " << ptr->path().string() << std::endl;
}

int main(int argc, char **argv)
{
    int exit_code = EXIT_SUCCESS;
//    try {

        fusecpp::directory root{"/"};
        print_path(root);
        root.create_directory("/Test_dir");
        root.create_file("/test_file");
        do_ls_entry(root);

        // Can we find root from root ?
        if (auto ptr = fusecpp::search_directory(root, "/"))
            std::cout << "Root found: " << ptr->path() << std::endl;
        else
            std::cout << "Root not found" << std::endl;

        // Test basic searching of directory
        if (auto ptr = fusecpp::search_directory(root, "/Test_dir"))
            std::cout << "/Test_dir found: " << ptr->path() << std::endl;
        else
            std::cout << "/Test_dir not found" << std::endl;

        // We are sure there is not /Test_dir/Sample_dir
        if (auto ptr = fusecpp::search_directory(root, "/Test_dir/Sample_dir"))
            std::cout << "/Test_dir/Sample_dir found: " << ptr->path() << std::endl;
        else
            std::cout << "/Test_dir/Sample_dir not found" << std::endl;

        // Let's add directory
        if (auto ptr = fusecpp::search_directory(root, "/Test_dir"))
            ptr->create_directory("/Test_dir/Sample_dir");

        // Search it again
        auto sample_dir = fusecpp::search_directory(root, "/Test_dir/Sample_dir");
        if (sample_dir)
            std::cout << "/Test_dir/Sample_dir found: " << sample_dir->path() << std::endl;
        else
            std::cout << "/Test_dir/Sample_dir not found" << std::endl;

        // Test adding file to the directory
        sample_dir->create_file("/Test_dir/Sample_dir/sample_file");
        // Can we find it ?
        if (auto ptr = fusecpp::search_file(root, "/Test_dir/Sample_dir/sample_file"))
            std::cout << "/Test_dir/Sample_dir/sample_file found: " << ptr->path() << std::endl;
        else
            std::cout << "/Test_dir/Sample_dir/sample_file not found" << std::endl;
        // OK. Now show you the all of entries
        do_ls_entry(*sample_dir);

/*    } catch (std::exception& e) {
        exit_code = EXIT_FAILURE;
        std::cerr << "[-] Exception: " << e.what() << std::endl;
    }*/
    return exit_code;
}
