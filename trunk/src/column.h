
class column_display
{
	public:
		column_display(int columns, char separator);
		void add(char * text, int column = 0);
		void dump();
		void clear();
		const int cols;
		const char sep;
	private:
		vector<string> flesh[5];
		
};

