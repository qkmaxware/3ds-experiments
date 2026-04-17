#include "../lib/ez3ds.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

class Explorer: public CitrusApp {
private:
    Imgui::FileBrowser fb;
    int page_size;
    int page_count;
    int page_index;
    bool error;
    std::string current_dir;
    
public:
    Explorer(): fb(), page_size(-1), page_count(-1), page_index(-1), error(false), current_dir() {}

    void setup() override {
        fb.SetDir("/");
    } 

    void loop(Displays &displays, Input &input) override { 
        // Clear the upper screen
        displays.Upper.Clear();

        // Browse
        fb.SelectFile(displays.Upper, input);

        // Get debug info
        int page_size = 0;
        int page_count = 0;
        int page_index = 0;
        fb.PageInfo(page_count, page_index, page_size);
        bool changed = page_size != this->page_size 
            || page_count != this->page_count 
            || page_index != this->page_index 
            || fb.HasErrorOccured() != this->error
            || fb.CurrentDirectory() != this->current_dir
        ;
        this->page_size = page_size;
        this->page_count = page_count;
        this->page_index = page_index;
        this->error = fb.HasErrorOccured();
        this->current_dir = fb.CurrentDirectory();

        // Draw some debug info on the lower screen if we desire
        if (changed) {
            Console::Clear();
            
            // Print some debug info
            if (fb.HasErrorOccured()) {
                Console::Println("An error has occurred fetching files");
                Console::Println("");
            }
            Console::Println("Current Directory: %s", fb.CurrentDirectory().c_str());
            Console::Println("Items in folder: %d", fb.FileCount());
            Console::Println("Page: %d/%d", page_index + 1, page_count);
            Console::Println("");
            Console::Println("Up/Down to navigate items, A to select a directory, B to go up a directory");
            Console::Println("Press START to quit");
        }
    }

    void cleanup() override {

  
    }
};

int main() {
    Explorer app;
    app.Run();
    return 0;
}