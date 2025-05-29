#include <bits/stdc++.h>
using namespace std;

int main() {
    while(true) {
        string input;
        getline(cin, input);
        if(input == "exit") return 0;

        istringstream stream(input);
        string op;
        stream>>op;
        
        if(op == "order") {
            // order 1 tata buy 100 100
            string userId, stockName, orderType;
            int price, quantity;
            stream>>userId>>stockName>>orderType>>price>>quantity;
        } else if(op == "cancel") {
            
        }
    }
    return 0;
}