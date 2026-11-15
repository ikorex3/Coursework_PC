#include "SearchApp.h"
#include <cstdlib>
using namespace std;

int main()
{
    const char* env_data = getenv("DATA_PATH");
    string data_path = env_data ? env_data:"./data";

    SearchApp app(8080, data_path);
    app.Run();
    return 0;
}
