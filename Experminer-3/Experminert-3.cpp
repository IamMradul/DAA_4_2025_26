#include <bits/stdc++.h>
using namespace std;

int main() {
    int n;
    cin >> n;

    vector<char> attendance(n);
    for (int i = 0; i < n; i++) {
        cin >> attendance[i];
    }

    unordered_map<int, int> mp;

    int sum = 0, m = 0;
    mp[0] = -1;

    for (int i = 0; i < n; i++) {
        if (attendance[i] == 'P')
            sum += 1;
        else
            sum -= 1;

        if (mp.find(sum) != mp.end()) {
            m = max(m, i - mp[sum]);
        } else {
            mp[sum] = i;
        }
    }

    cout << m << endl;
    return 0;
}
