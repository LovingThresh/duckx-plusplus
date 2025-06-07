#include <duckx.hpp>
#include <iostream>
using namespace std;

int main()
{
    try
    {
        auto doc = duckx::Document::open("my_test.docx");

        std::cout << "--- Document Content (using for-each loop) ---" << std::endl;

        for (auto& p : doc.body().paragraphs())
        {
            for (auto& r : p.runs())
            {
                std::cout << r.get_text();
            }
            std::cout << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
