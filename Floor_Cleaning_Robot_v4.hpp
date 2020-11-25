#include<iostream>
#include<fstream>
#include<string>
#include<queue>
#include<vector>
using namespace std;

enum tile_type {floor = 0, wall = 1, recharge = 2, unknown = 3};
const int direction[4][2] = {{-1,0},{0,1},{0,-1},{1,0}};
clock_t start, last;
ofstream outfile("final.path",ios::out);
int CharToType(char c);

struct position{
    int row;
    int col;

    position(int r=-1, int c=-1):row(r), col(c){}
    bool operator!=(position &other){
        return (row != other.row) || (col != other.col);
    }
    bool operator==(position &other){
        return (row == other.row) && (col == other.col);
    }
    //0=右，1=左，2=上，3=下
    position stepwise_move(int dir){
        return position(row+direction[dir][0],col+direction[dir][1]);
    }
};

struct trio{
    position pos;
    int step;
    vector<position> related;
    trio(position p = position(), int s=-1, vector<position> v = vector<position>());
};

class tile{
public:
    int minstep;
    int type;
    bool cleaned;
    int search_visited;
    position pos;
    vector<position> related;
public:
    tile(){}
    void set(int t, int row, int col);
    void clean();
};
bool tile_compare(const tile& a, const tile& b);

class tile_map{
public:
    tile **map;
    position Rpos;
    stack<position> todo;
    int cols, rows;
    int B;
    int walkable_num;
    int search_id;
public:
    tile_map(ifstream infile);
    bool is_walkable(position pos);
    void calculate_minstep();
    tile& get_tile(position p, const char* origin);
    void print_out(int t);
    trio find_uncleaned(position init, int battery);
};

class robot{
public:
    int maxbattery;
    int battery;
    position Rpos;
    position pos;
    vector<position> footprint;
public:
    robot();
    void walk();
    void jump();
    void hop();
    bool is_on_recharge();
    void print_out(ofstream& ofs);
};