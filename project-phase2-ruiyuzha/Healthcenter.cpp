#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#define Healthcenter_TCP_PORT 6955 //6000 + 955 (USCID: 4057818955)
#define Hospital1_UDP_PORT 22055 //21100 + 955 (USCID: 4057818955)
#define Hospital2_UDP_PORT 22155 //21200 + 955 (USCID: 4057818955)
#define Hospital3_UDP_PORT 22255 //21300 + 955 (USCID: 4057818955)
#define Student1_UDP_PORT 22355 //21400 + 955 (USCID: 4057818955)
#define Student2_UDP_PORT 22455 //21500 + 955 (USCID: 4057818955)
#define Student3_UDP_PORT 22555 //21600 + 955 (USCID: 4057818955)
#define Student4_UDP_PORT 22655 //21700 + 955 (USCID: 4057818955)
#define Student5_UDP_PORT 22755 //21800 + 955 (USCID: 4057818955)
#define Num_of_Hospitals 3
#define Num_of_Students 5
#define Num_of_Departments 3
#define BACKLOG 10	 // how many pending connections queue will hold
#define MAXDATASIZE 100 // max number of bytes we can get at once

using namespace std;

int tcp_fd, udp_fd;
struct sockaddr_in tcp_saddr, tcp_caddr;
struct sockaddr_in udp_saddr, udp_caddr;

void saveData(string information, string output_filename){
    ofstream fileout(output_filename, ios::out | ios::app);
    fileout << information << endl;
    fileout.close();
}

void create_TCP_server(){
    char IPaddr[INET_ADDRSTRLEN];
    struct hostent *he;

    /* Create a socket */
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd < 0) {
      perror("ERROR creating socket");
      exit(1);
   }
   
   memset(&tcp_saddr, '\0', sizeof(tcp_saddr)); // zero structure out
   tcp_saddr.sin_family = AF_INET; // match the socket() call
   tcp_saddr.sin_addr.s_addr = htonl(INADDR_ANY); // bind to any local address
   tcp_saddr.sin_port = htons(Healthcenter_TCP_PORT); // specify port to listen on

   if ((he = gethostbyname("www.bing.com")) == NULL) {
       perror("gethostbyname");
    }

   if (bind(tcp_fd, (struct sockaddr *) &tcp_saddr, sizeof(tcp_saddr)) < 0) { //bind
       perror("ERROR binding");
   }

   if(listen(tcp_fd, 5) < 0) { // listen for incoming connections
        perror("ERROR listening");
   }

   strcpy(IPaddr, inet_ntoa(*(struct in_addr*)he->h_addr));
   
   cout << "The health center has TCP port " << Healthcenter_TCP_PORT << " and IP address " << IPaddr << endl;
}

void TCP_recieve_hospital_information(){   
    int new_fd, pid;
    char buf[MAXDATASIZE];
    int numbytes;
    
    socklen_t clilen = sizeof(tcp_caddr);

    for(int i = 0; i < Num_of_Hospitals; i++){
        new_fd = accept(tcp_fd, (struct sockaddr *) &tcp_caddr, &clilen);

        if (new_fd < 0) {
            perror("ERROR creating socket");
            exit(1);
        }

        pid = fork();
        if (pid < 0)
            perror("ERROR on fork");
        if (pid == 0)  {
            close(tcp_fd);
            string str = "";
            for (int i = 0; i < Num_of_Departments+1; i++){
                if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) > 0) {  
                    str += buf;
                }
            }
            string output_filename = "Healthcenter_Hospital.txt";
            saveData(string(str), output_filename);
            string hospital = string(str).substr(0,9);
            cout << "Recieved the department list from <" << hospital << ">" << endl;
            exit(0);
        }
        else{
            close(new_fd);
        }    
    }
    sleep(5);
    cout << "End of Phase I for the health center" << endl;
}

void TCP_recieve_student_information(){
    int new_fd, pid, numbytes;
    char buf[MAXDATASIZE];
    char IPaddr[INET_ADDRSTRLEN];
    struct hostent *he;
    memset(buf, 0, MAXDATASIZE);
    socklen_t clilen = sizeof(tcp_caddr);

    if ((he = gethostbyname("www.bing.com")) == NULL) {
       perror("gethostbyname");
    }
    strcpy(IPaddr, inet_ntoa(*(struct in_addr*)he->h_addr));
    cout << "The health center has TCP port " << Healthcenter_TCP_PORT << " and IP address " << IPaddr << endl;

    for(int i=0; i < Num_of_Students; i++){
        new_fd = accept(tcp_fd, (struct sockaddr *) &tcp_caddr, &clilen);

        if (new_fd < 0) {
            perror("ERROR creating new_socket");
            exit(1);
        }

        pid = fork();
        if (pid < 0)
            perror("ERROR on fork");
        if (pid == 0) {
            close(tcp_fd);
            if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) > 0) {  
                sleep(1);
            }

            string output_filename = "Healthcenter_Student.txt";
            saveData(string(buf), output_filename);
            string student = string(buf).substr(0,8);
            cout << "health center receive the application from <" << student << ">" << endl;
            exit(0);
        }
        else{
            close(new_fd);
        }
    }
    sleep(5);
}

vector<string> readData(string filename){
    vector<string> data;
    string l;
    ifstream infile(filename);
    if(infile.is_open()){
        while(infile >> l){
            data.push_back(l);
        }
    }
    return data;
}

vector<vector<string> > read_student_information(vector <string> data){
    vector<vector<string> > result;
    result.resize(5);
    string str[]={"Symptom:", "Interest1:", "Interest2:", "Interest3:"};
    vector<string> flag(str, str+4);
    string student, symptom, interest;
    
    for (int l = 0; l < data.size(); l++){
        int position = 0;
        student = data[l].substr(0,8);
        result[l].push_back(student);
        position = data[l].find(flag[0],position);
        symptom = data[l].substr(position+flag[0].length(), 1);
        result[l].push_back(symptom);
        for (int f = 1; f < flag.size(); f++){
            position = data[l].find(flag[f],position);
            if(position>0){
                interest = data[l].substr(position+flag[f].length(), 2);
                result[l].push_back(interest);
            }
        }        
    }
    
    return result;
}

vector<vector<string> > read_hospital_information(vector<string> data){
    vector<vector<string> > result;
    result.resize(3);
    string flag1, flag2, flag3;
    for (int l = 0; l < data.size(); l++){
        int position1 = 0; 
        int position2 = 0;
        int position3 = 0;
        result[l].push_back(data[l].substr(0,9));
        flag1 = data[l].substr(8,1)+"1";
        flag2 = data[l].substr(8,1)+"2";
        flag3 = data[l].substr(8,1)+"3";
        position1 = data[l].find(flag1,position1);
        position2 = data[l].find(flag2,position2);
        position3 = data[l].find(flag3,position3);
        
        result[l].push_back(flag1);
        result[l].push_back(data[l].substr(position1+3, position2-position1-3));
        result[l].push_back(flag2);
        result[l].push_back(data[l].substr(position2+3, position3-position2-3));
        result[l].push_back(flag3);
        result[l].push_back(data[l].substr(position3+3));
    }

    return result;
}

bool check_sympton(string symptom, string range){
    int position = 0;
    string flag = ",";
    position = range.find(flag,position);
    int low = stoi(range.substr(0,position));
    int high = stoi(range.substr(position+1));
    int val = stoi(symptom);
    if (val <= high && val >= low){
        return 1;
    }else{
        return 0;
    }
}

string find_hospital(string interest, string symptom, vector<vector <string> > res2){
    int l;
    string str = interest.substr(0,1);
    string res_department;
    if(str == "A"){
        l = 0;
    }
    else if(str == "B"){
        l = 1;
    }
    else{
        l = 2;
    }
    
    if(res2[l][1] == interest && check_sympton(symptom, res2[l][2])){
        res_department = res2[l][1];
    }else if(res2[l][3] == interest && check_sympton(symptom, res2[l][4])){        
        res_department = res2[l][3];
    }else if(res2[l][5] == interest && check_sympton(symptom, res2[l][6])){
        res_department = res2[l][5];
    }else{
        res_department = "Reject";
    }
    return res_department;
}

string get_student_result(vector<vector <string> > res1, vector<vector <string> > res2, int num){
    vector<string> application = res1[num];
  
    string student = application[0];
    string symptom = application[1];
    string interest;
    string res_department, res_hospital, res;
    int i = 2;
    res = "Reject";

    while(i < application.size()){
        interest = application[i];
        res_department =find_hospital(interest, symptom, res2);
        if(res_department != "Reject"){
            res_hospital = "hospital" + res_department.substr(0,1);
            res = "Accept#" + res_department + "#" + res_hospital;
            break;
        }
        i++;      
    }
    return res;
}

int get_hospital_port(string data){
    int port = 0;
    if(data != "Reject"){
        string s = data.substr(10);
        if(s == "hospitalA"){
            port = Hospital1_UDP_PORT;
        }else if(s == "hospitalB"){
            port = Hospital2_UDP_PORT;
        }else{
            port = Hospital3_UDP_PORT;
        }
    }
    return port;
}

string get_hospital_result(vector<vector <string> > res1, string res_student, string num){
    string res;
    int l = stoi(num)-1;
    if(res_student != "Reject"){
        string s1 = "Student" + num;
        string s2 = res1[l][1];
        string s3 = res_student.substr(7,2);
        res = s1 + "#" + s2 + "#" + s3;
    }
    return res;
}

void UDP_send_reply_to_student(string data, int num, int port){
    struct hostent *he;
    char buf[MAXDATASIZE];
    char IPaddr[INET_ADDRSTRLEN];
    int numbyte;

    if ((he = gethostbyname("www.google.com")) == NULL) {
       perror("gethostbyname");
    }
    struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;

    memset(&udp_saddr, '\0', sizeof(udp_saddr)); // zero structure out
    udp_saddr.sin_family = AF_INET;
    udp_saddr.sin_port = htons(port);
    udp_saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    strcpy(IPaddr, inet_ntoa(udp_saddr.sin_addr));
    
    strcpy(buf, data.c_str());
    sendto(udp_fd, buf, MAXDATASIZE, 0, (const struct sockaddr *) &udp_saddr, sizeof(udp_caddr));
    
    bzero(&udp_caddr, sizeof(udp_caddr));
    socklen_t len = sizeof(udp_saddr);
    getsockname(udp_fd, (struct sockaddr *) &udp_caddr, &len);
    inet_ntop(AF_INET, &udp_caddr.sin_addr, IPaddr, sizeof(IPaddr));
    int Port = ntohs(udp_caddr.sin_port);
    if (num == 1)
        cout << "The health center has UDP port " << Port << " and IP address " << IPaddr << " for Phase 2" << endl;
    
    cout << "The health center has send the application result to <Student" << num << ">" <<endl;
    sleep(1);
}

void UDP_send_result_to_hospital(string data, int num, int port){
    struct hostent *he;
    char buf[MAXDATASIZE];
    char IPaddr[INET_ADDRSTRLEN];
    int numbyte;

    if ((he = gethostbyname("www.google.com")) == NULL) {
       perror("gethostbyname");
    }
    struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;

    memset(&udp_saddr, '\0', sizeof(udp_saddr));
    udp_saddr.sin_family = AF_INET;
    udp_saddr.sin_port = htons(port);
    udp_saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    strcpy(IPaddr, inet_ntoa(udp_saddr.sin_addr));
    
    strcpy(buf, data.c_str());
    sendto(udp_fd, buf, MAXDATASIZE, 0, (const struct sockaddr *) &udp_saddr, sizeof(udp_caddr));
    
    bzero(&udp_caddr, sizeof(udp_caddr));
    socklen_t len = sizeof(udp_saddr);
    getsockname(udp_fd, (struct sockaddr *) &udp_caddr, &len);
    inet_ntop(AF_INET, &udp_caddr.sin_addr, IPaddr, sizeof(IPaddr));
    int Port = ntohs(udp_caddr.sin_port);
    
    string hospital = data.substr(11,1);
    cout << "The health center has send one admitted student to <Hospital" << hospital << ">" <<endl;
    sleep(1);
}

int main(void) {
    create_TCP_server();
    TCP_recieve_hospital_information();

    TCP_recieve_student_information();

    vector<string> student_information = readData("Healthcenter_Student.txt");
    vector<vector<string> > res1 = read_student_information(student_information);
    vector<string> hospital_information = readData("Healthcenter_Hospital.txt");
    vector<vector<string> > res2 = read_hospital_information(hospital_information);

    /* Create a socket */
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
      perror("ERROR creating socket");
      exit(1);
    }
    
    int port;

    string res_student1 = get_student_result(res1, res2, 0);
    UDP_send_reply_to_student(res_student1, 1, Student1_UDP_PORT);
    string res_hospital_s1 = get_hospital_result(res1, res_student1, "1");
    port = get_hospital_port(res_student1);
    UDP_send_result_to_hospital(res_hospital_s1, 1, port);

    string res_student2 = get_student_result(res1, res2, 1);
    UDP_send_reply_to_student(res_student2, 2, Student2_UDP_PORT);
    string res_hospital_s2 = get_hospital_result(res1, res_student2, "2");
    port = get_hospital_port(res_student2);
    UDP_send_result_to_hospital(res_hospital_s2, 2, port);

    string res_student3 = get_student_result(res1, res2, 2);
    UDP_send_reply_to_student(res_student3, 3, Student3_UDP_PORT);
    string res_hospital_s3 = get_hospital_result(res1, res_student3, "3");
    port = get_hospital_port(res_student3);
    UDP_send_result_to_hospital(res_hospital_s3, 3, port);

    string res_student4 = get_student_result(res1, res2, 3);
    UDP_send_reply_to_student(res_student4, 4, Student4_UDP_PORT);
    string res_hospital_s4 = get_hospital_result(res1, res_student4, "4");
    port = get_hospital_port(res_student4);
    UDP_send_result_to_hospital(res_hospital_s4, 4, port);

    string res_student5 = get_student_result(res1, res2, 4);
    UDP_send_reply_to_student(res_student5, 5, Student5_UDP_PORT);
    string res_hospital_s5 = get_hospital_result(res1, res_student5, "5");
    port = get_hospital_port(res_student5);
    UDP_send_result_to_hospital(res_hospital_s5, 5, port);
    
    return 0;
}