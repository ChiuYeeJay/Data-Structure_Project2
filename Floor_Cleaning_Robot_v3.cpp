#include<iostream>
#include<fstream>
#include<string>
#include<queue>
#include<vector>
#include<stack>
#include"Floor_Cleaning_Robot_v3.hpp"
using namespace std;

// enum tile_type {floor = 0, wall = 1, recharge = 2, unknown = 3};
const int direction[4][2] = {{0,1},{0,-1},{1,0},{-1,0}};
tile_map* TileMap;

int CharToType(char c){
    switch(c){
        case '0': return floor;
        case '1': return wall;
        case 'R': return recharge;
        default : return unknown;
    }
}

//class quartet
quartet::quartet(int r, int c, int s, vector<position> v){
    row = r;
    col = c;
    step = s;
    related = v;
}


//class tile
tile::tile(int t, int row, int col){
    set(t,row,col);
}
void tile::set(int t, int row, int col){
    minstep = -1;
    cleaned = false;
    type = t;
    pos = position(row,col);
}
void tile::clean(){
    if(cleaned){
        printf("Error: tile clean more than once\n");
        exit(7);
    }
    cleaned = true;
}


//class tile_map
tile_map::tile_map(ifstream infile){
    char temp;
    infile >> rows >> cols >> B;
    Rpos.col = Rpos.row = -1;
    map = new tile*[rows];
    for(int i=0;i<rows;i++){
        map[i] = new tile[cols];
        for(int j=0;j<cols;j++){
            infile >> temp;
            temp = CharToType(temp);
            if(temp == unknown){
                printf("Error: unknown tile type\n");
                exit(2);
            }
            else if(temp == recharge){
                if(Rpos.col + Rpos.row != -2){
                    printf("Error: multipe charges\n");
                    exit(3);
                }
                else{
                    Rpos.row = i;
                    Rpos.col = j;
                }
            }
            map[i][j].set(temp,i,j);
        }
    }
    calculate_minstep();
}
bool tile_map::is_walkable(int r, int c){
    if(r<0 || r>=rows) return false;
    if(c<0 || r>=cols) return false;
    if(map[r][c].type == wall) return false;
    return true;
}

void tile_map::calculate_minstep(){
    queue<quartet> Q;
    quartet tp;
    vector<position> vp;
    while(!Q.empty()) Q.pop();

    Q.push(quartet(Rpos.row,Rpos.col,0));
    get_tile(Rpos,"root clean").cleaned = true;
    while(!Q.empty()){
        tp = Q.front();
        vp = tp.related;
        vp.push_back(position(tp.row,tp.col));
        if(map[tp.row][tp.col].minstep == -1){
            get_tile(position(tp.row,tp.col),"calculate_minstep1").minstep = tp.step;
            get_tile(position(tp.row,tp.col),"calculate_minstep2").related = vp;
            if(is_walkable(tp.row+1,tp.col)) Q.push(quartet(tp.row+1,tp.col,tp.step+1,vp));
            if(is_walkable(tp.row-1,tp.col)) Q.push(quartet(tp.row-1,tp.col,tp.step+1,vp));
            if(is_walkable(tp.row,tp.col+1)) Q.push(quartet(tp.row,tp.col+1,tp.step+1,vp));
            if(is_walkable(tp.row,tp.col-1)) Q.push(quartet(tp.row,tp.col-1,tp.step+1,vp));
            todo.push(position(tp.row,tp.col));
        }
        Q.pop();
        // printf("h\n");
    }
}

tile& tile_map::get_tile(position p, const char* origin){
    if(p.row<0 || p.row>=rows || p.col<0 || p.col>=cols){
        printf("Error: get_tile failed, from %s\n", origin);
        exit(6);
    }
    // printf("get_tile(%d,%d):%d\n", p.row, p.col, map[p.row][p.col].cleaned);
    return map[p.row][p.col];
}
void tile_map::print_out(int t){
    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            if(t == 1) printf("%d ", map[i][j].type);
            else if(t == 2) printf("%3d ", map[i][j].minstep);
            else{
                if(is_walkable(i,j)) printf("%c ", map[i][j].cleaned? '.' : '+');
                else printf("M ");
            }
        }
        printf("\n");
    }
}


//class robot
robot::robot(){
    pos = Rpos = TileMap->Rpos;
    maxbattery = battery = TileMap->B;
    while(!footprint.empty()) footprint.pop();
    footprint.push(Rpos);
}
void robot::jump(){
    tile_map &tm = *TileMap;
    if(tm.todo.empty()){
        printf("Error: jump when todo stack empty\n");
        exit(8);
    }
    tile &target = tm.get_tile(tm.todo.top(),"robot jump2");
    tm.todo.pop();
    for(position p:target.related){
        footprint.push(p);
        if(!tm.get_tile(p,"robot jump3").cleaned) tm.get_tile(p,"robot jump3").clean();
    }
    pos = target.pos;
    battery -= target.related.size();
}
void robot::walk(){
    tile_map &tm = *TileMap;
    position best, tpos;
    //找到適合的移動方向
    for(int i=0;i<4;i++){
        tpos = position(pos.row+direction[i][0], pos.col+direction[i][1]);
        if(tm.is_walkable(tpos.row, tpos.col) && tm.get_tile(tpos,"robot walk1").minstep <= battery-1){
            if(best.row == -1 || tile_compare(tm.get_tile(tpos,"robot walk2"), tm.get_tile(best,"robot walk3"))){
                best = tpos;
            }
        }
    }
    if(best.row == -1){
        printf("Error: robot cannot move, battery=%d\n", battery);
        exit(4);
    }
    //移動至最好的位置
    pos = best;
    footprint.push(best);
    if(!tm.get_tile(best,"robot walk4").cleaned) tm.get_tile(best,"robot walk4.5").clean();
    //減少電量
    if(is_on_recharge()) battery = maxbattery;
    else battery--;
    if(battery <= 0){
        printf("Error: robot battery exhausted\n");
        exit(5);
    }
}
bool robot::tile_compare(const tile& a, const tile& b){
    if(a.cleaned && !b.cleaned) return false;
    else if(!a.cleaned && b.cleaned) return true;
    else if(a.minstep > b.minstep) return true;
    else return false;
}
bool robot::is_on_recharge(){
    return pos == Rpos;
}
void robot::print_out(ofstream& ofs){
    ofs << footprint.size() - 1l << endl;
    while(!footprint.empty()){
        ofs << footprint.front().row << " " << footprint.front().col << endl;
        footprint.pop();
    }
}

void DEBUG(robot &R){
    string d;
    static int count = 0;
    static bool stop = false;
    static int maxround;
    printf("#(%d,%d), battery=%d, footprintsize=%ld, todosize=%ld\n", R.pos.row, R.pos.col,\
     R.battery, R.footprint.size(), TileMap->todo.size());
    // if(!stop) cin >> d;
    if(!stop) d = "a";
    else{
        if(maxround > count){
            count++;
            TileMap->print_out(3);
        }
        else{
            stop = false;
            maxround = 0;
            count = 0;
        }
    }

    if(d == "a"){
        stop = true;
        maxround = 10;
        count = 0;
    }
    else if(d == "b"){
        stop = true;
        maxround = 100;
        count = 0;
    }
    else if(d == "c"){
        stop = true;
        maxround = 1000;
        count = 0;
    }
    else if(d == "p") TileMap->print_out(3);
}

int main(int argc, char *argv[]){
    string debug;
    if(argc != 2) {
        printf("Error: amount of file in file out is wrong\n");
        exit(1);
    }
    ofstream outfile("final.path",ios::out);
    TileMap = new tile_map(ifstream(argv[1],ios::in));
    robot* Robot = new robot();
    // TileMap->print_out(2);
    while(!TileMap->todo.empty()){
        Robot->jump();
        while(!Robot->is_on_recharge()) Robot->walk();
        while(!TileMap->todo.empty() && TileMap->get_tile(TileMap->todo.top(),"robot jump1").cleaned) TileMap->todo.pop();
        // DEBUG(*Robot);
    }
    // TileMap->print_out(3);
    Robot->print_out(outfile);
}