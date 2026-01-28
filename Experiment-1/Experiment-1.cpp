//Name- Mradul Gupta
//UID- 24BCS10693

#include <bits/stdc++.h>

using namespace std;

using namespace std::chrono;

int dpth=0;

void complexRec(int n) {

    dpth++;

   if (n <= 2) {

       cout<<dpth<<endl;

       return;

   }

   int p = n;

   while (p > 0) {

       vector<int> temp(n);

       for (int i = 0; i < n; i++) {

           temp[i] = i ^ p;

       }

       p >>= 1;

   }

   vector<int> small(n);

   for (int i = 0; i < n; i++) {

       small[i] = i * i;

   }

   if (n % 3 == 0) {

       reverse(small.begin(), small.end());

   } else {

       reverse(small.begin(), small.end());

   }

   complexRec(n / 2);

   complexRec(n / 2);

   complexRec(n / 2);

}

int main() {

    int n;

    cin>>n;

    auto start = high_resolution_clock::now();

    complexRec(n);

    auto end = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(end - start);

    cout << "Recursion Depth = " << dpth << endl;
    cout << "Time  = " << duration.count() << endl;

    

    return 0;

}

//Recursion Relation =>  f(n)=3T(n/2)+n * logn

//Time Complexity => O(n^log(2,3))      

