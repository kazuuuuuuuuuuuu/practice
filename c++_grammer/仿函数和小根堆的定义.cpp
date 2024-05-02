class Solution {
public:
    // 仿函数
    struct compare
    {
        // 重定义调用运算符
        bool operator()(vector<int> &a, vector<int>&b)
        {
            return a[1] > b[1];
        }
    };
    vector<int> topKFrequent(vector<int>& nums, int k) 
    {
        unordered_map<int, int> map;
        for(int num : nums)
        {
            map[num] ++;
        }

        priority_queue<vector<int>, vector<vector<int>>, compare> min_heap;
        for(auto[key, value] : map)
        {
            vector<int> next = {key, value};
            min_heap.push(next);
            if(min_heap.size()>k)
                min_heap.pop();
        }

        vector<int> ans(k);
        for(int i=k-1;i>=0;i--)
        {
            ans[i] = min_heap.top()[0];
            min_heap.pop();
        }
        return ans;
    }
};