class Solution {
public:
    int minEatingSpeed(vector<int>& piles, int h) {
        int st=1;
        int end= *max_element(piles.begin(), piles.end());

        while(st<=end){
            int mid=st + (end-st)/2;
            long long totalhr=0;

            for(int pile: piles){
                totalhr+= (pile + mid- 1)/mid;
            }
            if(totalhr <= h) end=mid-1;

            else st=mid+1;
        }
        return st;
    }
};