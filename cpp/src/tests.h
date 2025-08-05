#include <string>
#include <vector>

using namespace std;

bool all_tests();

bool general_test(int num_coeff, string data, vector<pair<int, int>> to_verify, vector<tuple<int, int, string>> to_refute);

bool main_test();
