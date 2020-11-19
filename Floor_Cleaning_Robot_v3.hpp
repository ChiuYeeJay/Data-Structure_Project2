#include<iostream>
#include<fstream>
#include<string>
#include<queue>
#include<vector>
using namespace std;

enum tile_type {floor = 0, wall = 1, recharge = 2, unknown = 3};
// const int direction[4][2] = {{0,1},{0,-1},{1,0},{-1,0}};
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
};

struct quartet{
    int row;
    int col;
    int step;
    vector<position> related;
    quartet(int r=-1, int c=-1, int s=-1, vector<position> v = vector<position>());
};

class tile{
public:
    int minstep;
    int type;
    bool cleaned;
    position pos;
    vector<position> related;
public:
    tile(int t = unknown, int row = -1, int col = -1);
    void set(int t, int row, int col);
    void clean();
};

class tile_map{
public:
    tile **map;
    position Rpos;
    stack<position> todo;
    int cols, rows;
    int B;
public:
    tile_map(ifstream infile);
    bool is_walkable(int r, int c);
    void calculate_minstep();
    tile& get_tile(position p, const char* origin);
    void print_out(int t);
};

class robot{
public:
    int maxbattery;
    int battery;
    position Rpos;
    position pos;
    queue<position> footprint;
public:
    robot();
    void walk();
    void jump();
    bool tile_compare(const tile& a, const tile& b);
    bool is_on_recharge();
    void print_out(ofstream& ofs);
};