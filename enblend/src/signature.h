// Compiled by the CyanogenMod Team
#define VERSION "CyanogenMod"
#define PACKAGE_BUGREPORT "xplodwild@cyanogenmod.org"
class Signature {
public:
	void initialize();
	bool check() { return true; }
	const char* message() { return "Enblend for Nemesis"; }
};