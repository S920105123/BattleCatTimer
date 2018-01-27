#include "cell_lib.h"

void CellLib::open(string filename){
    File_Reader in;
    LOG(NORMAL) << "[CellLib] is parsing ..." << filename << endl;
    in.open(filename);

    string token, name;
    do{
        token = in.next_token();
        if(token=="lu_table_template"){
            EXPECT(in.next_token(), "(");
            name = in.next_token();
            EXPECT(in.next_token(), ")");
            LuTableTemplate *table = new LuTableTemplate();
            table->set_name(name);
            table->read(in);
            add_table_template(name, table);
        }
        else if(token=="cell"){
            EXPECT(in.next_token(), "(");
            name = in.next_token();
            EXPECT(in.next_token(), ")");
            Cell *cell = new Cell();
            cell->set_name(name);
            cell->read(in);
            add_cell(name, cell);
        }
    }while(!token.empty());
}

void CellLib::add_table_template(string name, LuTableTemplate* table){
    if(table_template.find(name)==table_template.end()){
        table_template[name] = table;
    }else LOG(ERROR) << "[CellLib][add_table_template] " << name << " appear twice.\n";
}

void CellLib::add_cell(string name, Cell* cell){
    if(cells.find(name)==cells.end()){
        cells[name] = cell;
    }else LOG(ERROR) << "[CellLib][add_cell] " << name << " appear twice.\n";
}

int CellLib::cells_size(){
    return cells.size();
}

int CellLib::table_template_size(){
    return table_template.size();
}

int CellLib::print_cell(string name){
    if(cells.find(name)==cells.end()){
        LOG(CERR) << " no such cells " << name << endl;
    }else cells[name]->print();
}

int CellLib::print_template(){
    for(auto i:table_template) i->print();
}
