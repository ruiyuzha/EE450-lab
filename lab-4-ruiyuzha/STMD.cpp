#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <utility>

using namespace std;

vector<string> readData(string filename){
    vector<string> data;
    string l;
    ifstream infile(filename);
    if(infile.is_open()){
        while(getline (infile, l)){
            data.push_back(l);
        }
    }
    return data;
}

vector<vector<string> > read_source_information(vector<string> data){
    vector<vector<string> > result;
    result.resize(5);
    string source;
    for (int l = 0; l < data.size()-1; l++){
        source = data[l].substr(0,7);
        result[l].push_back(source);
        int num = count(data[l].begin(),data[l].end(),',');
        int len = (data[l].size()-8-num)/6;
        for (int i = 0; i < len; i++){
            result[l].push_back(data[l].substr(8+7*i, 6));
        }
    }
    return result;
}

string read_source_priority(vector<string> data){
    string result;
    result = data[data.size()-1];
    return result;
}

int find_duration(vector<vector<string> > data){
    int duration = 0;
    vector <int> tmp;
    for (int l = 0; l < data.size(); l++){
        for (int i = 1; i < data[l].size(); i++){
            int end_time = stoi(data[l][i].substr(2,1));
            tmp.push_back(end_time);
        }
    }
    return *max_element(tmp.begin(),tmp.end());;
}

vector<vector<int> > create_input_buffer(vector<vector<string> > data, int duration){
    vector<vector<int> > inputBuf;
    inputBuf.resize(5);
    for (int k = 0; k < 5; k++){
        inputBuf[k].resize(duration+1);
    }
    for (int i = 0; i < 5; i++){
        for (int j = 1; j < data[i].size(); j++){
            int start_time = stoi(data[i][j].substr(0,1));
            int end_time = stoi(data[i][j].substr(2,1));
            int source = stoi(data[i][j].substr(5,1));
            for (int k = start_time; k < end_time; k++){
                inputBuf[i][k] = source;
            } 
        }
    }
    return inputBuf;
}

int main(int argc, char* argv[]){
    if(argc != 2) {
        cout << "Please enter only one file after " << argv[0] << " ." << endl;
        return 1;
    }
    vector<string> input = readData(argv[1]);
    vector<vector<string> > source_information = read_source_information(input);
    string source_priority = read_source_priority(input);
    int duration = find_duration(source_information);

    vector<vector<int> > inputBuf = create_input_buffer(source_information, duration);

    for (int i = 0; i < inputBuf.size();i++){
        for(int j =0; j< inputBuf[i].size();j++){
            cout << inputBuf[i][j] << " ";
        }
        cout << endl;
    }

    int total_packet = 0;
    for (int i = 0; i < inputBuf.size();i++){
        for(int j =0; j< inputBuf[i].size();j++){
            if(inputBuf[i][j] != 0){
                total_packet+=1;
            }
        }
    }
    
    double transmission_rate = total_packet/duration;
    
    int n = source_priority.length(); 
    char char_array[n + 1]; 
    strcpy(char_array, source_priority.c_str());

    int a;
    double f=0;
    cout << "SF" << endl;
    for (int i = 0; i < inputBuf[0].size();i++){
        for (int k = 0; k < (n+1)/2; k++){
            a = char_array[2*k]-'A';
            if (inputBuf[a][i] != 0){    
                if (inputBuf[a][i] != inputBuf[a][i+1]){
                    if(inputBuf[a][i] != inputBuf[a][i-1]){
                        cout << a << " ";
                        cout << f << " ";
                        f = f+(1/transmission_rate);
                        cout << f << " ";
                        cout << char_array[2*k] << inputBuf[a][i] << endl; 
                    }            
                }
                else{
                    cout << a << " ";
                    cout << f << " ";
                    f = f+2*(1/transmission_rate);
                    cout << f << " ";
                    cout << char_array[2*k] << inputBuf[a][i] << endl;
                }
            }
        }
        
    }
    cout << "EF" << endl;
}