#include "xstring.h"
#include <fstream>
#include <sstream>
#include "disp.h"
#include <complex.h>


//class StringProcessor
//{
//public:
//	void flip(x::string& str, int n)
//	{
//		for (int i = 0; i < str.size_; ++i)
//			str.data_[i] += n;
//	}
//
//	void flip_char(x::string& str, x::string const& key)
//	{
//		for (int i = 0; i < str.size_; ++i)
//			str.data_[i] += key.data_[i % key.size_];
//	}
//
//	void flip_char(x::string& str, x::string const& key, int mult)
//	{
//		for (int i = 0; i < str.size_; ++i)
//			str.data_[i] += mult*key.data_[i % key.size_];
//	}
//
//	
//};

long long unsigned num_hash(char const* str, size_t size)
{
	long long unsigned num = 0;
	char* snum = reinterpret_cast<char*>(&num);
	static constexpr size_t SIZE = sizeof(long long unsigned);
	for (int i = 0; i < size; ++i) {
		num += str[i];
		num *= (size+11);
		num <<= 1;
		snum[(i + str[(i - 1) % size]) % SIZE] += (str[i] <<(i%2));
	}
	return num;
}

char* hash1(const char* str, size_t size, size_t hashsize)
{
	char* hash = new char[hashsize + 1]{0};
	long long unsigned strnum = num_hash(str, size);
	
	for (size_t i = 0; i < hashsize; ++i) {
		hash[i] += str[i%size]* (i^size);
		hash[i] += (hash[(i - 1) % hashsize] ^ strnum << (8 * (i % sizeof(decltype(strnum)))));
	}
	return hash;
}

class RecspHasher
{
	char const* str_;
	size_t size_;
	size_t hashsize_;
	char* hash_;
	size_t gtsize_;

	void process_(size_t p1, size_t p2)
	{
		size_t half = (p1 + p2) / 2;
		if (p2 > p1) {
			process_(p1, half);
			process_(half + 1, p2);
		}
		else
			hash_[p1] = (str_[p1%size_] ^ str_[(p1 + 1) % size_] + gtsize_/hashsize_) * size_;
		//displn hash_;
		for (size_t i = p1; i <= p2; ++i) {
			hash_[i] += hash_[(i - 1) % hashsize_] ^ hash_[(i + 1) % hashsize_];
		}
	}

public:
	RecspHasher(char const* str, size_t size, size_t hashsize):
		str_{str},
		size_{size},
		hashsize_{hashsize},
		hash_{new char[hashsize + 1]{0}},
		gtsize_{x::max(size_, hashsize_)}
	{
	}

	char* generate()
	{
		process_(0, hashsize_ - 1);
		return hash_;
	}
};


char* recsp(char const* str, size_t size, size_t hashsize)
{
	return RecspHasher{str, size, hashsize}.generate();
}

char* hash(const char* str, size_t size, size_t hashsize, size_t loops = 10)
{
	char* hash = new char[hashsize + 1]{0};
	char* strc = new char[size + 1]{0};
	memcpy(strc, str, size);

	for (size_t i = 0; i < hashsize; ++i)
		hash[i] = str[i%size];
	//displn hash;

	size_t begin = str[0] % size;
	size_t hi = 0;

	long long strnum = num_hash(strc, size);

	//displn strnum;

	repeat(loops)
	{
		for (size_t i = begin, c = 0; c < size; ++c, (++i) %= size) {
			//str[i] += str[(i + str[i]) % size];
			strc[i] += strc[strc[(i + strc[i] << (i % 8) - strc[size - i - 1]) % size] % size];
			hash[(++hi) %= hashsize] += strc[i];
		}
		begin = str[begin] % size;
		//displn hash;
	}
	delete[] strc;
	return hash;
}

void next(char* str, size_t size)
{
	for (int i = 0; i < size; ) {
		if ((++str[i]) == 0) ++i;
		else break;
	}
}

class Encrypter
{
	size_t filesize(const char* filename)
	{
		std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
		return in.tellg();
	}

	//x::string read(std::ifstream& inFile)
	//{
	//	//if (inFile) {
	//		std::ostringstream contents;
	//		contents << inFile.rdbuf();
	//		inFile.close();
	//		return contents.str();
	//	//}
	//	//throw "Invalid input file.";
	//}

	x::string read(char const* fileName)
	{
		size_t fileSize = filesize(fileName);
		std::ifstream inFile{fileName, std::ios::binary | std::ios::in};
		char* buffer = new char[fileSize];
		inFile.read(buffer, fileSize);
		inFile.close();
		return x::string(buffer, fileSize);
		//}
		//throw "Invalid input file.";
	}

	x::string hashPassword(x::string password)
	{
		return x::string{hash(password.data(), password.size(), 128), 128};
	}


public:
	void encrypt(char* inFileName, char* outFileName, x::string const& password)
	{
		x::string passhash = hashPassword(password);
		std::ofstream outFile{outFileName, std::ios::binary | std::ios::out};
		x::string in{read(inFileName)};
		x::string key{(size_t)1024, true};
		key.randomize(0, 255);

		in.flip_char(key);
		key.flip_char(passhash);

		outFile.write(key.data(),key.size());
		outFile.write(in.data(), in.size());
		outFile.close();
	}

	void decrypt(char* inFileName, char* outFileName, x::string const& password)
	{
		x::string passhash = hashPassword(password);
		std::ofstream outFile{outFileName, std::ios::binary | std::ios::out};
		x::string in{read(inFileName)};
		x::string key = in.substr(0, 1023);

		key.flip_char(passhash, -1);
		in.flip_char(key, -1);

		outFile.write(in.data() + 1024, in.size()-1024);
		outFile.close();
	}
};

int main1(int argCount, char** args)
{
	auto hash_fn = recsp;

	/*auto str1 = "password";
	auto str2 = "pbssword";
	auto str3 = "passwnrd";
	auto str4 = "assword";
	auto str5 = "passwordq";
	auto str6 = "passwoord";*/

	auto str1 = "66666666666666666";
	auto str2 = "6666666666666666";
	auto str3 = "666666666666666";
	auto str4 = "66666666666666";
	auto str5 = "6666666666666";
	auto str6 = "666666666666";

	/*displn num_hash(str1, 8);
	displn num_hash(str2, 8);
	displn num_hash(str3, 8);
	displn num_hash(str4, 7);
	displn num_hash(str5, 9);
	displn num_hash(str6, 9);*/

	displn hash_fn(str1, 17, 17 );
	displn hash_fn(str2, 16, 16 );
	displn hash_fn(str3, 15, 15 );
	displn hash_fn(str4, 14, 14 );
	displn hash_fn(str5, 13, 13 );
	displn hash_fn(str6, 12, 12 );
	//return 0;
	//auto password = "password";
	//x::string spassword = "password";
	//x::string passhash{hash(password, spassword.size(), 128, 1000), 1000};
	////disp password,"\n\n";
	//x::string passhash2{hash(password, spassword.size(), 128, 1000), 1000};
	//x::string passhash3{hash(password, spassword.size(), 128, 1000), 1000};

	//displn passhash;
	//displn passhash2;
	//displn passhash3;

	//return 0;
	//displn hash("D:\\VSProjects\\Project3\\Debug\\in.txt", 35, 64, 1000);
	//displn hash("D:\\VSProjects\\Project3\\Debug\\in.txt", 35, 64, 1000);
	//return 0;
	//Encrypter e;
	//e.encrypt("D:\\VSProjects\\Project3\\Debug\\in.txt", "D:\\VSProjects\\Project3\\Debug\\out.txt", "password");
	////Sleep(1000);
	//e.decrypt("D:\\VSProjects\\Project3\\Debug\\out.txt", "D:\\VSProjects\\Project3\\Debug\\dec.txt", "password");

	size_t SIZE = 2;
	size_t HASHSIZE = 2;
	long long N = pow(256LL, SIZE);

	//x::string infile = args[1];
	//x::string outfile = args[2];

	//char* str = new char[10]{"krzysztof"};
	char* str = new char[SIZE + 1]{0};
	x::vector<x::string> hv(N);
	
	
	repeat(N){
		next(str, SIZE);
		hv.push_back({hash(str,SIZE,HASHSIZE),HASHSIZE});
	}

	displn hv.size();
	hv.unify();
	displn hv.size();
	system("pause");
}



///////////////////////////////////////////////////////////////////////////////

using byte = unsigned char;

class InheritanceTable
{
	static constexpr size_t CLASS_NUMBER_ = 10;

	static byte inheritance_[CLASS_NUMBER_][CLASS_NUMBER_];
	static size_t classNumber_;


public:
	static void set(size_t typeNumber)
	{
		inheritance_[typeNumber][typeNumber] = 1;
	}

	static void inherit(size_t what, size_t from)
	{
		inheritance_[what][from] = 1;
		
	}
};

byte InheritanceTable::inheritance_[InheritanceTable::CLASS_NUMBER_][InheritanceTable::CLASS_NUMBER_] = {0};
size_t InheritanceTable::classNumber_ = InheritanceTable::CLASS_NUMBER_;

int main()
{

}