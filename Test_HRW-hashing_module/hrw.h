//#include <string>
//#include <vector>

using namespace std;

vector<pair<string, uint32_t>> order_HRW(const vector<string>& ids, string& key);
vector<string> convert_ids_to_strings_with_id(vector<int> ids, string word);
vector<int> convert_strings_with_id_to_ids(vector<string> strings_with_id);
int extractNumber(string& input);