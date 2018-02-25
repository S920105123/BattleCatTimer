// #include "cell_lib.h"
#include "liberty.h"

void CellLib::open(const string& filename){
    File_Reader in;
    LOG(NORMAL) << "[CellLib] is parsing ..." << filename << '\n';
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
            Cell *cell = new Cell(this);
            cell->set_name(name);
            cell->read(in);
            add_cell(name, cell);
        }
    }while(!token.empty());
}

void CellLib::add_table_template(const string& name, LuTableTemplate* table){
    if(table_template.find(name)==table_template.end()){
        table_template[name] = table;
    }else LOG(ERROR) << "[CellLib][add_table_template] " << name << " appear twice.\n";
}

void CellLib::add_cell(const string& name, Cell* cell){
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

void CellLib::print_cell(const string& name){
    if(cells.find(name)==cells.end()){
        LOG(CERR) << " no such cells " << name << '\n';
    }else cells[name]->print();
}

void CellLib::print_template(){
    for (auto &i : table_template) i.second->print();
}

float CellLib::get_pin_capacitance(const string& cell_type, const string& pin_name){
    if(cells.find(cell_type)==cells.end()){
        LOG(ERROR) << "[CellLib][get_pin_capacitance] no such cell type: " <<  cell_type << '\n';
    }else return cells[cell_type]->get_pin_capacitance(pin_name);
    return 0;
}

bool CellLib::get_pin_is_clock(const string& cell_type, const string& pin_name){
    if(cells.find(cell_type)==cells.end()){
        LOG(ERROR) << "[CellLib][get_pin_is_clock] no such cell type: " <<  cell_type << '\n';
    }else return cells[cell_type]->get_pin_is_clock(pin_name);
    return false;
}

Direction_type CellLib::get_pin_direction(const string& cell_type, const string& pin_name){
    if(cells.find(cell_type)==cells.end()){
        LOG(ERROR) << "[CellLib][get_pin_direction] no such cell type: " <<  cell_type << '\n';
    } else {
    	return cells[cell_type]->get_pin_direction(pin_name);
	}
    return OUTPUT; // Unknown
}

vector<TimingArc*>* CellLib::get_pin_TimingArc(const string& cell_type, const string& pin_name, const string& src){
    if(cells.find(cell_type)==cells.end()){
        LOG(ERROR) << "[CellLib][get_pin_TimingArc] no such cell type: " <<  cell_type << '\n';
    }else return cells[cell_type]->get_pin_TimingArc(pin_name, src);
    return NULL;
}

vector<TimingArc*>* CellLib::get_pin_total_TimingArc(const string& cell_type, const string& pin_name){
    if(cells.find(cell_type)==cells.end()){
        LOG(ERROR) << "[CellLib][get_pin_total_TimingArc] no such cell type: " <<  cell_type << '\n';
    }else return cells[cell_type]->get_pin_total_TimingArc(pin_name);
    return NULL;
}

Cell* CellLib::get_cell_ptr(const string& cell_type){
    if(cells.find(cell_type)==cells.end()){
        LOG(ERROR) << "[CellLib][get_cell_ptr] no such cell type: " <<  cell_type << '\n';
    }else return cells[cell_type];
    return NULL;
}

Pin* CellLib::get_pin_ptr(const string& cell_type, const string& pin_name){
    if(cells.find(cell_type)==cells.end()){
        LOG(ERROR) << "[CellLib][get_pin_ptr] no such cell type: " <<  cell_type << '\n';
    }else return cells[cell_type]->get_pin_ptr(pin_name);
    return NULL;
}

LuTableTemplate* CellLib::get_table_template(const string& table_label){
    if(table_template.find(table_label)==table_template.end()){
        LOG(ERROR) << "[CellLib][get_table_template] no such table_label: " <<  table_label << '\n';
    }else return table_template[table_label];
    return NULL;
}

/* -- Test -- */
#ifdef TEST_CELLLIB

int main()
{
	string filename;
	CellLib lib;
	cout << "enter filename : ";
	cin  >> filename;
    lib.open(filename);
	cout << "open " << filename << "ok\n";

	cout << "total cells: " << lib.cells_size() << '\n';
	cout << "template table : ";
	lib.print_template();

	string name;
    while(true){
		cout << "enter cell type or exit : ";
		cin >> name;
		if(name=="exit") break;
		lib.print_cell(name);
    }
	Logger* logger = Logger::create();
    logger->~Logger();
    return 0;
}

#endif /* end TEST_CELLLIB */
