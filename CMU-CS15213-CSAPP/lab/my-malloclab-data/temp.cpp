#include <bits/stdc++.h>
using namespace std;
int high_size(int x) {
	size_t p = 1;
	while (p < x) { p <<= 1; }
	return p;
}
int q[100000];
int main() {
	freopen("1.txt", "w", stdout);
	freopen("2.txt", "r", stdin);
	string s;
	int line = 0, ans = 0;
	while (getline(cin, s)) {
		line++;
		stringstream ss(s);
		string a;
		ss >> a;
		int x, y;
		if (a == "a") {
			ss >> x >> y;
			q[x] = high_size(y);
			ans += q[x];
			// cout << x << " " << ans << endl;
		} else if (a == "f") {
			ss >> x;
			ans -= q[x];
		}
		cout << a << " " << x << " " << ans << endl;
	}
	fclose(stdin);
	fclose(stdout);
}