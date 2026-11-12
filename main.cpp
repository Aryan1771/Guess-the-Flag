#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <random>
#include <ctime>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <memory>
#include <unordered_set>
#include <numeric>
#include <cctype>
#include <cwctype>
#include <stdexcept>
#include <cstdlib>
#include <utility>
#include <map>
#include <bcrypt.h>
#include <cstring>

#define IDI_APP 101

using namespace Gdiplus;
using std::wstring;

namespace {
constexpr int W = 1120, H = 760;
constexpr size_t ROUND_COUNT = 10;
constexpr size_t ANSWER_COUNT = 4;
const Color BG(255, 29, 32, 29), PANEL(255, 43, 47, 42), PANEL2(255, 55, 62, 53);
const Color UI_TEXT(255, 245, 239, 221), MUTED(255, 181, 184, 168), ACCENT(255, 111, 184, 153);
const Color RED(255, 218, 126, 101), GREEN(255, 112, 189, 137), LINE(255, 82, 91, 77);
enum class Screen { Account, Home, Playing, Result, GameOver, Leaderboard, ForgotPassword, Recovery };
enum class GameMode { Standard, Endless, Timed };
struct Country {
 std::string code;
 std::wstring name;
 std::filesystem::path imagePath;
 std::unique_ptr<Image> flagImage;
};
std::vector<Country> countries;
std::wstring loadError, loadWarnings;
struct Box { int x,y,w,h; bool has(int a,int b) const { return a>=x&&a<x+w&&b>=y&&b<y+h; } };
HWND hwnd; ULONG_PTR gdip; HICON smallAppIcon=nullptr; Screen screen=Screen::Account,previousScreen=Screen::Home; GameMode selectedMode=GameMode::Standard,previousMode=GameMode::Standard; std::mt19937 rng((unsigned)std::time(nullptr));
std::vector<int> rounds, choices, flagDeck; size_t deckIndex=0; int roundNo=0, score=0, selected=-1, hover=-1, hoverMode=-1; bool locked=false, trackingMouse=false, hoverFinish=false, userFocused=true,passwordFocused=false,recoveryUserFocused=false,recoveryKeyFocused=false,newPasswordFocused=false,confirmPasswordFocused=false,showPassword=false,caretVisible=true,resetFromLogin=true;
std::wstring playerName, userInput, passwordInput, recoveryCode, recoveryInput, newPasswordInput, confirmPasswordInput, profileError, authError, answerFeedback;
size_t userCaret=0,passwordCaret=0,recoveryCaret=0,newPasswordCaret=0,confirmPasswordCaret=0;
int highScore=0,lastScore=0,streak=0,maxStreak=0,correctCount=0,mistakes=0,lastAward=0,timeLimitSeconds=300,previousTimeLimit=300,remainingSeconds=300;
ULONGLONG questionStart=0,gameStart=0;
Box standardModeBox{50,475,325,105}, endlessModeBox{398,475,325,105}, timedModeBox{746,475,325,105};
Box userNameBox{260,300,600,50}, passwordBox{260,385,600,50}, loginBtn{260,465,280,54}, createAccountBtn{580,465,280,54};
Box loginEyeBox{812,388,42,44}, forgotPasswordBtn{260,526,280,28};
Box recoveryUserBox{260,300,600,46}, recoveryKeyBox{260,369,600,46}, recoveryNewPasswordBox{260,438,548,46}, recoveryConfirmPasswordBox{260,507,548,46};
Box recoveryKeyEyeBox{812,370,42,44}, recoveryNewEyeBox{812,439,42,44}, recoveryConfirmEyeBox{812,508,42,44};
Box resetPasswordBtn{600,590,260,54}, forgotBackBtn{260,590,260,54}, copyRecoveryBtn{260,610,260,54}, recoveryContinueBtn{600,610,260,54}, changePasswordBtn{595,35,150,44};
Box accountPrev{870,552,38,26}, accountNext{912,552,38,26};
std::array<Box,4> accountRowBoxes{{{260,580,600,30},{260,614,600,30},{260,648,600,30},{260,682,600,30}}};
Box timer5Box{405,594,95,34}, timer2Box{515,594,95,34}, timer1Box{625,594,95,34};
Box startBtn{390,640,340,55}, nextBtn{746,615,250,56}, finishBtn{930,24,130,38}, homeBtn{70,650,220,56}, playAgainBtn{720,650,330,56};
Box leaderboardBtn{760,35,150,44}, logoutBtn{920,35,140,42}, backBtn{70,680,220,52};
Box resultLeaderboardBtn{390,650,260,56};
std::array<Box,5> leaderboardModeBoxes{{{70,190,185,46},{268,190,185,46},{466,190,185,46},{664,190,185,46},{862,190,185,46}}};
struct ScoreRecord{int high=0,last=0;};
struct BoardRow{std::wstring user;int score=0;};
std::vector<std::wstring> accounts;
size_t accountOffset=0;
std::map<std::string,ScoreRecord> scoreRecords;
std::vector<BoardRow> boardRows;
std::string boardKey;
std::array<Box,ANSWER_COUNT> optionBoxes{{{383,480,350,62},{747,480,350,62},{383,554,350,62},{747,554,350,62}}};

void brush(Graphics& g, Color c, int x,int y,int w,int h){ SolidBrush b(c); g.FillRectangle(&b,x,y,w,h); }
void roundPath(GraphicsPath& p,int x,int y,int w,int h,int r){p.Reset();p.AddArc(x,y,2*r,2*r,180,90);p.AddArc(x+w-2*r,y,2*r,2*r,270,90);p.AddArc(x+w-2*r,y+h-2*r,2*r,2*r,0,90);p.AddArc(x,y+h-2*r,2*r,2*r,90,90);p.CloseFigure();}
void rounded(Graphics& g, Color c, int x,int y,int w,int h,int r=16){ GraphicsPath p;roundPath(p,x,y,w,h,r);SolidBrush b(c);g.FillPath(&b,&p); }
void outline(Graphics& g, Color c, int x,int y,int w,int h,int r=16,float sw=1){ GraphicsPath p;roundPath(p,x,y,w,h,r);Pen pen(c,sw);g.DrawPath(&pen,&p); }
std::string trimAscii(std::string value){
 const auto first=value.find_first_not_of(" \t\r\n");
 if(first==std::string::npos)return {};
 const auto last=value.find_last_not_of(" \t\r\n");
 return value.substr(first,last-first+1);
}
std::wstring utf8ToWide(const std::string& value){
 if(value.empty())return {};
 const int size=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,value.data(),static_cast<int>(value.size()),nullptr,0);
 if(size<=0)throw std::runtime_error("invalid UTF-8");
 std::wstring result(static_cast<size_t>(size),L'\0');
 if(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,value.data(),static_cast<int>(value.size()),result.data(),size)!=size)
  throw std::runtime_error("UTF-8 conversion failed");
 return result;
}
std::filesystem::path executableDirectory(){
 std::wstring path(32768,L'\0');
 const DWORD length=GetModuleFileNameW(nullptr,path.data(),static_cast<DWORD>(path.size()));
 if(length==0||length>=path.size())throw std::runtime_error("Could not locate the running executable.");
 path.resize(length);
 return std::filesystem::path(path).parent_path();
}
std::string wideToUtf8(const std::wstring& value){
 if(value.empty())return {};
 const int size=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,value.data(),static_cast<int>(value.size()),nullptr,0,nullptr,nullptr);
 if(size<=0)return {};
 std::string result(static_cast<size_t>(size),'\0');
 WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,value.data(),static_cast<int>(value.size()),result.data(),size,nullptr,nullptr);
 return result;
}
std::filesystem::path saveDirectory(){return executableDirectory()/L"profiles";}
std::wstring trimWide(std::wstring value){
 const auto first=value.find_first_not_of(L" \t\r\n");if(first==std::wstring::npos)return {};
 const auto last=value.find_last_not_of(L" \t\r\n");return value.substr(first,last-first+1);
}
std::wstring canonicalName(std::wstring value){
 value=trimWide(std::move(value));for(auto& ch:value){if(ch>=L'A'&&ch<=L'Z')ch=static_cast<wchar_t>(ch-L'A'+L'a');}
 return value;
}
bool validUsername(const std::wstring& name){
 if(name.size()<3||name.size()>20)return false;
 return std::all_of(name.begin(),name.end(),[](wchar_t ch){return (ch>=L'a'&&ch<=L'z')||(ch>=L'A'&&ch<=L'Z')||(ch>=L'0'&&ch<=L'9')||ch==L'_'||ch==L'-';});
}
std::filesystem::path accountPath(const std::wstring& name){return saveDirectory()/(canonicalName(name)+L".acct");}
std::filesystem::path scorePath(const std::wstring& name){return saveDirectory()/(canonicalName(name)+L".scores");}
std::string recordKey(){
 if(selectedMode==GameMode::Standard)return "standard";
 if(selectedMode==GameMode::Endless)return "endless";
 return "timed_"+std::to_string(timeLimitSeconds);
}
std::string hexEncode(const unsigned char* data,size_t length){
 static constexpr char digits[]="0123456789abcdef";std::string out;out.reserve(length*2);
 for(size_t i=0;i<length;i++){out.push_back(digits[data[i]>>4]);out.push_back(digits[data[i]&15]);}return out;
}
bool hexDecode(const std::string& text,unsigned char* out,size_t length){
 if(text.size()!=length*2)return false;
 auto nibble=[](char ch)->int{if(ch>='0'&&ch<='9')return ch-'0';if(ch>='a'&&ch<='f')return ch-'a'+10;if(ch>='A'&&ch<='F')return ch-'A'+10;return -1;};
 for(size_t i=0;i<length;i++){int hi=nibble(text[i*2]),lo=nibble(text[i*2+1]);if(hi<0||lo<0)return false;out[i]=static_cast<unsigned char>((hi<<4)|lo);}return true;
}
bool derivePasswordHash(const std::wstring& password,const unsigned char* salt,unsigned char* hash){
 std::string bytes=wideToUtf8(password);BCRYPT_ALG_HANDLE algorithm=nullptr;
 if(BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,BCRYPT_ALG_HANDLE_HMAC_FLAG)!=0){if(!bytes.empty())SecureZeroMemory(bytes.data(),bytes.size());return false;}
 const NTSTATUS status=BCryptDeriveKeyPBKDF2(algorithm,reinterpret_cast<PUCHAR>(bytes.data()),static_cast<ULONG>(bytes.size()),const_cast<PUCHAR>(salt),16,210000,hash,32,0);
 if(!bytes.empty())SecureZeroMemory(bytes.data(),bytes.size());BCryptCloseAlgorithmProvider(algorithm,0);return status>=0;
}
void clearPassword(){if(!passwordInput.empty())SecureZeroMemory(passwordInput.data(),passwordInput.size()*sizeof(wchar_t));passwordInput.clear();}
void clearRecoveryFields(){
 for(auto* value:{&recoveryInput,&newPasswordInput,&confirmPasswordInput})if(!value->empty())SecureZeroMemory(value->data(),value->size()*sizeof(wchar_t));
 recoveryInput.clear();newPasswordInput.clear();confirmPasswordInput.clear();
}
bool makeRecoveryCredential(std::wstring& code,std::array<unsigned char,16>& salt,std::array<unsigned char,32>& hash){
 std::array<unsigned char,16> randomCode{};
 if(BCryptGenRandom(nullptr,salt.data(),static_cast<ULONG>(salt.size()),BCRYPT_USE_SYSTEM_PREFERRED_RNG)<0||BCryptGenRandom(nullptr,randomCode.data(),static_cast<ULONG>(randomCode.size()),BCRYPT_USE_SYSTEM_PREFERRED_RNG)<0)return false;
 std::string encoded=hexEncode(randomCode.data(),randomCode.size());std::transform(encoded.begin(),encoded.end(),encoded.begin(),[](unsigned char ch){return static_cast<char>(std::toupper(ch));});
 code=utf8ToWide(encoded);return derivePasswordHash(code,salt.data(),hash.data());
}
std::wstring normalizeRecoveryCode(std::wstring value){
 std::wstring result;for(wchar_t ch:value){if(ch==L' '||ch==L'-')continue;if(ch>=L'a'&&ch<=L'f')ch=static_cast<wchar_t>(ch-L'a'+L'A');result.push_back(ch);}return result;
}
bool writeAccountFile(const std::wstring& name,const std::array<unsigned char,16>& passwordSalt,const std::array<unsigned char,32>& passwordHash,const std::array<unsigned char,16>& recoverySalt,const std::array<unsigned char,32>& recoveryHash){
 std::ofstream file(accountPath(name),std::ios::binary|std::ios::trunc);if(!file)return false;
 file<<wideToUtf8(name)<<'\n'<<hexEncode(passwordSalt.data(),passwordSalt.size())<<'\n'<<hexEncode(passwordHash.data(),passwordHash.size())<<'\n'<<hexEncode(recoverySalt.data(),recoverySalt.size())<<'\n'<<hexEncode(recoveryHash.data(),recoveryHash.size())<<'\n';
 file.close();return static_cast<bool>(file);
}
bool constantTimeEqual(const unsigned char* a,const unsigned char* b,size_t length){unsigned int diff=0;for(size_t i=0;i<length;i++)diff|=a[i]^b[i];return diff==0;}
void loadAccounts(){
 accounts.clear();accountOffset=0;std::error_code ec;const auto dir=saveDirectory();
 if(!std::filesystem::exists(dir,ec))return;
 for(const auto& entry:std::filesystem::directory_iterator(dir,ec)){
  if(ec)break;if(entry.path().extension()!=L".acct")continue;
  std::ifstream file(entry.path(),std::ios::binary);std::string line;
  if(std::getline(file,line)){try{std::wstring name=utf8ToWide(line);if(validUsername(name))accounts.push_back(name);}catch(...){}}
 }
 std::sort(accounts.begin(),accounts.end(),[](const auto& a,const auto& b){return canonicalName(a)<canonicalName(b);});
}
void readScoreFile(const std::wstring& name,std::map<std::string,ScoreRecord>& target){
 target.clear();std::ifstream file(scorePath(name),std::ios::binary);std::string line;
 while(std::getline(file,line)){std::istringstream row(line);std::string key,high,last;if(std::getline(row,key,'|')&&std::getline(row,high,'|')&&std::getline(row,last,'|'))target[key]={std::max(0,std::atoi(high.c_str())),std::max(0,std::atoi(last.c_str()))};}
}
void loadProfile(){
 profileError.clear();readScoreFile(playerName,scoreRecords);
 const auto it=scoreRecords.find(recordKey());highScore=it==scoreRecords.end()?0:it->second.high;lastScore=it==scoreRecords.end()?0:it->second.last;
}
void saveProfile(){
 try{
  auto& record=scoreRecords[recordKey()];record.last=score;record.high=std::max(record.high,score);highScore=record.high;lastScore=record.last;
  std::filesystem::create_directories(saveDirectory());std::ofstream file(scorePath(playerName),std::ios::binary|std::ios::trunc);
  if(!file){profileError=L"Scores could not be saved. Check that the profiles folder is writable.";return;}
  for(const auto& item:scoreRecords)file<<item.first<<'|'<<item.second.high<<'|'<<item.second.last<<'\n';
  if(!file)profileError=L"Scores could not be saved. Check that the profiles folder is writable.";else profileError.clear();
 }catch(const std::exception&){profileError=L"Scores could not be saved. Check that the profiles folder is writable.";}
}
bool createAccount(){
 const auto name=trimWide(userInput);
 if(!validUsername(name)){authError=L"Use 3–20 letters, numbers, _ or - for the account name.";return false;}
 if(passwordInput.size()<8||passwordInput.size()>128){authError=L"Choose a password between 8 and 128 characters.";return false;}
 try{
  std::filesystem::create_directories(saveDirectory());if(std::filesystem::exists(accountPath(name))){authError=L"That account already exists. Sign in instead.";return false;}
  std::array<unsigned char,16> salt{},recoverySalt{};std::array<unsigned char,32> hash{},recoveryHash{};
  if(BCryptGenRandom(nullptr,salt.data(),static_cast<ULONG>(salt.size()),BCRYPT_USE_SYSTEM_PREFERRED_RNG)<0||!derivePasswordHash(passwordInput,salt.data(),hash.data())||!makeRecoveryCredential(recoveryCode,recoverySalt,recoveryHash)){authError=L"Secure account storage could not be initialized.";return false;}
  if(!writeAccountFile(name,salt,hash,recoverySalt,recoveryHash)){authError=L"Could not write the account file.";return false;}
  playerName=name;authError.clear();loadAccounts();scoreRecords.clear();
  const auto legacyPath=executableDirectory()/L"saves"/(canonicalName(name)+L".sav");std::ifstream legacy(legacyPath,std::ios::binary);std::string legacyName,legacyHigh,legacyLast;
  if(std::getline(legacy,legacyName)&&std::getline(legacy,legacyHigh)&&std::getline(legacy,legacyLast)){
   scoreRecords["standard"]={std::max(0,std::atoi(legacyHigh.c_str())),std::max(0,std::atoi(legacyLast.c_str()))};
   std::ofstream migrated(scorePath(name),std::ios::binary|std::ios::trunc);migrated<<"standard|"<<scoreRecords["standard"].high<<'|'<<scoreRecords["standard"].last<<'\n';migrated.close();
  }
  loadProfile();screen=Screen::Recovery;clearPassword();userFocused=passwordFocused=false;return true;
 }catch(const std::exception&){authError=L"Could not create this account.";return false;}
}
bool signIn(){
 const auto name=trimWide(userInput);
 if(!validUsername(name)){authError=L"Enter your account name (3–20 letters, numbers, _ or -).";return false;}
 if(passwordInput.empty()){authError=L"Enter your password to sign in.";return false;}
 try{
  std::ifstream file(accountPath(name),std::ios::binary);std::string storedName,saltText,hashText,recoverySaltText,recoveryHashText;
  if(!file||!std::getline(file,storedName)||!std::getline(file,saltText)||!std::getline(file,hashText)){authError=L"No saved account found with that name.";return false;}
  std::array<unsigned char,16> salt{};std::array<unsigned char,32> expected{},actual{};
  if(!hexDecode(saltText,salt.data(),salt.size())||!hexDecode(hashText,expected.data(),expected.size())||!derivePasswordHash(passwordInput,salt.data(),actual.data())||!constantTimeEqual(expected.data(),actual.data(),expected.size())){authError=L"That password doesn’t match this account.";return false;}
  std::array<unsigned char,16> passwordSalt{},recoverySalt{};std::array<unsigned char,32> passwordHash{},recoveryHash{};
  if(!hexDecode(saltText,passwordSalt.data(),passwordSalt.size())||!hexDecode(hashText,passwordHash.data(),passwordHash.size())){authError=L"This account file is damaged.";return false;}
  const bool hasRecovery=std::getline(file,recoverySaltText)&&std::getline(file,recoveryHashText)&&hexDecode(recoverySaltText,recoverySalt.data(),recoverySalt.size())&&hexDecode(recoveryHashText,recoveryHash.data(),recoveryHash.size());file.close();
  playerName=name;authError.clear();loadProfile();clearPassword();userFocused=passwordFocused=false;
  if(hasRecovery){screen=Screen::Home;return true;}
  if(!makeRecoveryCredential(recoveryCode,recoverySalt,recoveryHash)||!writeAccountFile(name,passwordSalt,passwordHash,recoverySalt,recoveryHash)){authError=L"Could not add a recovery key to this account.";return false;}
  screen=Screen::Recovery;return true;
 }catch(const std::exception&){authError=L"Could not read this account file.";return false;}
}
bool resetAccountPassword(){
 const auto name=trimWide(userInput);if(!validUsername(name)){authError=L"Enter the account name you want to recover.";return false;}
 if(newPasswordInput.size()<8||newPasswordInput.size()>128){authError=L"Choose a new password between 8 and 128 characters.";return false;}
 if(newPasswordInput!=confirmPasswordInput){authError=L"Those new passwords do not match.";return false;}
 const auto code=normalizeRecoveryCode(recoveryInput);if(code.size()!=32){authError=L"Enter the 32-character recovery code saved for this account.";return false;}
 try{
  std::ifstream file(accountPath(name),std::ios::binary);std::string storedName,passwordSaltText,passwordHashText,recoverySaltText,recoveryHashText;
  if(!file||!std::getline(file,storedName)||!std::getline(file,passwordSaltText)||!std::getline(file,passwordHashText)||!std::getline(file,recoverySaltText)||!std::getline(file,recoveryHashText)){authError=L"This account has no recovery key. Sign in and create one first.";return false;}
  std::array<unsigned char,16> oldSalt{},savedRecoverySalt{},newSalt{};std::array<unsigned char,32> oldHash{},savedRecoveryHash{},enteredRecoveryHash{},newHash{};
  if(!hexDecode(passwordSaltText,oldSalt.data(),oldSalt.size())||!hexDecode(passwordHashText,oldHash.data(),oldHash.size())||!hexDecode(recoverySaltText,savedRecoverySalt.data(),savedRecoverySalt.size())||!hexDecode(recoveryHashText,savedRecoveryHash.data(),savedRecoveryHash.size())||!derivePasswordHash(code,savedRecoverySalt.data(),enteredRecoveryHash.data())||!constantTimeEqual(savedRecoveryHash.data(),enteredRecoveryHash.data(),savedRecoveryHash.size())){authError=L"That recovery code does not match this account.";return false;}
  file.close();
  if(BCryptGenRandom(nullptr,newSalt.data(),static_cast<ULONG>(newSalt.size()),BCRYPT_USE_SYSTEM_PREFERRED_RNG)<0||!derivePasswordHash(newPasswordInput,newSalt.data(),newHash.data())||!writeAccountFile(name,newSalt,newHash,savedRecoverySalt,savedRecoveryHash)){authError=L"Could not save the new password securely.";return false;}
  playerName=name;loadProfile();clearPassword();clearRecoveryFields();showPassword=false;authError.clear();resetFromLogin=false;screen=Screen::Home;userFocused=passwordFocused=false;return true;
 }catch(const std::exception&){authError=L"Could not update this account.";return false;}
}
void loadLeaderboard(){
 boardRows.clear();boardKey=recordKey();std::error_code ec;const auto dir=saveDirectory();if(!std::filesystem::exists(dir,ec))return;
 for(const auto& entry:std::filesystem::directory_iterator(dir,ec)){
  if(ec)break;if(entry.path().extension()!=L".scores")continue;
  std::wstring name=entry.path().stem().wstring();std::ifstream accountFile(accountPath(name),std::ios::binary);std::string displayName;if(std::getline(accountFile,displayName)){try{name=utf8ToWide(displayName);}catch(...){}}
  std::map<std::string,ScoreRecord> records;readScoreFile(name,records);
  const auto it=records.find(boardKey);if(it!=records.end()&&it->second.high>0)boardRows.push_back({name,it->second.high});
 }
 std::sort(boardRows.begin(),boardRows.end(),[](const auto& a,const auto& b){return a.score!=b.score?a.score>b.score:canonicalName(a.user)<canonicalName(b.user);});
 if(boardRows.size()>10)boardRows.resize(10);
}
bool loadCountries(){
 countries.clear();loadError.clear();loadWarnings.clear();
 std::filesystem::path root;
 try{root=executableDirectory();}
 catch(const std::exception&){loadError=L"Guess the Flag could not locate its executable folder.";return false;}
 const std::filesystem::path manifestPath=root/L"assets"/L"flags"/L"flags.txt";
 std::ifstream manifest(manifestPath,std::ios::binary);
 if(!manifest){loadError=L"The country manifest could not be opened:\n"+manifestPath.wstring()+L"\n\nKeep the assets folder beside GuessTheFlag.exe.";return false;}

 std::unordered_set<std::string> seenCodes;
 std::vector<std::wstring> issueExamples;
 size_t lineNumber=0,skipped=0;
 auto issue=[&](size_t line,const std::wstring& reason){
  ++skipped;
  if(issueExamples.size()<8)issueExamples.push_back(L"Line "+std::to_wstring(line)+L": "+reason);
 };
 std::string line;
 while(std::getline(manifest,line)){
  ++lineNumber;
  if(lineNumber==1&&line.size()>=3&&static_cast<unsigned char>(line[0])==0xEF&&static_cast<unsigned char>(line[1])==0xBB&&static_cast<unsigned char>(line[2])==0xBF)line.erase(0,3);
  line=trimAscii(line);
  if(line.empty()||line[0]=='#')continue;
  const size_t first=line.find('|'),second=first==std::string::npos?first:line.find('|',first+1);
  if(first==std::string::npos||second==std::string::npos||line.find('|',second+1)!=std::string::npos){issue(lineNumber,L"Expected CODE|COUNTRY_NAME|IMAGE_PATH.");continue;}
  const std::string code=trimAscii(line.substr(0,first));
  const std::string nameBytes=trimAscii(line.substr(first+1,second-first-1));
  const std::string imageBytes=trimAscii(line.substr(second+1));
  if(code.size()!=2||!std::all_of(code.begin(),code.end(),[](unsigned char ch){return ch>='A'&&ch<='Z';})||nameBytes.empty()||imageBytes.empty()){
   issue(lineNumber,L"Country code, name, or image path is invalid.");continue;
  }
  if(seenCodes.find(code)!=seenCodes.end()){issue(lineNumber,L"Duplicate country code "+utf8ToWide(code)+L".");continue;}
  std::wstring wideName;
  std::filesystem::path relativePath;
  try{wideName=utf8ToWide(nameBytes);relativePath=std::filesystem::path(utf8ToWide(imageBytes));}
  catch(const std::exception&){issue(lineNumber,L"Country name or image path is not valid UTF-8.");continue;}
  if(relativePath.empty()||relativePath.is_absolute()){
   issue(lineNumber,L"Image paths must be relative to the game executable.");continue;
  }
  std::filesystem::path fullPath=(root/relativePath).lexically_normal();
  std::error_code ec;
  if(!std::filesystem::is_regular_file(fullPath,ec)){
   issue(lineNumber,L"Missing image for "+wideName+L" ("+fullPath.wstring()+L").");continue;
  }
  std::wstring extension=fullPath.extension().wstring();
  std::transform(extension.begin(),extension.end(),extension.begin(),[](wchar_t ch){return static_cast<wchar_t>(towlower(ch));});
  if(extension!=L".png") {issue(lineNumber,L"The flag image must be a PNG: "+fullPath.wstring());continue;}
  std::ifstream png(fullPath,std::ios::binary);
  std::array<unsigned char,8> signature{};
  png.read(reinterpret_cast<char*>(signature.data()),static_cast<std::streamsize>(signature.size()));
  const std::array<unsigned char,8> pngSignature{{137,80,78,71,13,10,26,10}};
  if(png.gcount()!=static_cast<std::streamsize>(pngSignature.size())||signature!=pngSignature){
   issue(lineNumber,L"Corrupt or non-PNG image for "+wideName+L".");continue;
  }
  std::unique_ptr<Image> image(Image::FromFile(fullPath.c_str(),FALSE));
  if(!image||image->GetLastStatus()!=Ok||image->GetWidth()==0||image->GetHeight()==0){
   issue(lineNumber,L"GDI+ could not decode the flag for "+wideName+L".");continue;
  }
  seenCodes.insert(code);
  countries.push_back(Country{code,std::move(wideName),fullPath,std::move(image)});
 }
 if(skipped){
  loadWarnings=L"Some flag entries were skipped ("+std::to_wstring(skipped)+L"):";
  for(const auto& example:issueExamples)loadWarnings+=L"\n\u2022 "+example;
  if(skipped>issueExamples.size())loadWarnings+=L"\n\u2022 ...and "+std::to_wstring(skipped-issueExamples.size())+L" more.";
 }
 if(countries.size()<ROUND_COUNT){
  loadError=L"Only "+std::to_wstring(countries.size())+L" usable flags were found. At least ten are needed to start a game.\n\nCheck assets/flags/flags.txt and its PNG files.";
  if(!loadWarnings.empty())loadError+=L"\n\n"+loadWarnings;
  return false;
 }
 return true;
}
void txt(Graphics& g, const wchar_t* s, int x,int y,int w,int h,float size,Color c,bool bold=false, int align=0){
 FontFamily fam(L"Segoe UI"); Font f(&fam,size,bold?FontStyleBold:FontStyleRegular,UnitPixel); SolidBrush b(c);
 StringFormat sf; sf.SetLineAlignment(StringAlignmentCenter); sf.SetAlignment(align==1?StringAlignmentCenter:align==2?StringAlignmentFar:StringAlignmentNear);
 RectF r((REAL)x,(REAL)y,(REAL)w,(REAL)h); g.DrawString(s,-1,&f,r,&sf,&b);
}
void txt(Graphics& g, const wstring& s, int x,int y,int w,int h,float size,Color c,bool bold=false,int align=0){txt(g,s.c_str(),x,y,w,h,size,c,bold,align);}
void flag(Graphics& g,int idx,int x,int y,int w,int h){
 if(idx<0||static_cast<size_t>(idx)>=countries.size()||w<=0||h<=0)return;
 const Country& country=countries[static_cast<size_t>(idx)];
 GraphicsState state=g.Save();
 GraphicsPath clip;roundPath(clip,x,y,w,h,10);g.SetClip(&clip,CombineModeIntersect);
 brush(g,PANEL2,x,y,w,h);
 if(country.flagImage){
  const REAL imageWidth=static_cast<REAL>(country.flagImage->GetWidth());
  const REAL imageHeight=static_cast<REAL>(country.flagImage->GetHeight());
  const REAL scale=std::min(static_cast<REAL>(w)/imageWidth,static_cast<REAL>(h)/imageHeight);
  const REAL drawWidth=imageWidth*scale,drawHeight=imageHeight*scale;
  RectF destination(static_cast<REAL>(x)+(w-drawWidth)/2.0f,static_cast<REAL>(y)+(h-drawHeight)/2.0f,drawWidth,drawHeight);
  g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
  g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
  g.DrawImage(country.flagImage.get(),destination,0,0,imageWidth,imageHeight,UnitPixel);
 }
 g.Restore(state);
 outline(g,Color(60,255,255,255),x,y,w,h,10,1);
}
void newGame(){
 if(countries.size()<ROUND_COUNT||playerName.empty())return;
 std::vector<int> pool(countries.size());std::iota(pool.begin(),pool.end(),0);std::shuffle(pool.begin(),pool.end(),rng);
 rounds.clear();choices.clear();flagDeck=std::move(pool);deckIndex=0;
 if(selectedMode==GameMode::Standard){rounds.assign(flagDeck.begin(),flagDeck.begin()+static_cast<std::ptrdiff_t>(ROUND_COUNT));deckIndex=ROUND_COUNT;}
 else rounds.push_back(flagDeck[deckIndex++]);
 roundNo=score=streak=maxStreak=correctCount=mistakes=lastAward=0;selected=-1;hover=-1;hoverMode=-1;locked=false;screen=Screen::Playing;
 gameStart=GetTickCount64();remainingSeconds=timeLimitSeconds;lastScore=0;questionStart=gameStart;saveProfile();
 if(selectedMode==GameMode::Timed)SetTimer(hwnd,2,250,nullptr);
}
void makeChoices(){
 if(roundNo<0||static_cast<size_t>(roundNo)>=rounds.size()||countries.size()<ANSWER_COUNT){choices.clear();locked=false;return;}
 choices.assign(1,rounds[static_cast<size_t>(roundNo)]);
 std::vector<int> pool;pool.reserve(countries.size()-1);
 for(size_t i=0;i<countries.size();++i)if(static_cast<int>(i)!=rounds[static_cast<size_t>(roundNo)])pool.push_back(static_cast<int>(i));
 std::shuffle(pool.begin(),pool.end(),rng);
 choices.insert(choices.end(),pool.begin(),pool.begin()+static_cast<std::ptrdiff_t>(ANSWER_COUNT-1));
 std::shuffle(choices.begin(),choices.end(),rng);selected=-1;locked=false;
}
void roundStart(){makeChoices();questionStart=GetTickCount64();InvalidateRect(hwnd,nullptr,FALSE);}
void beginGame(){
 const auto first=playerName.find_first_not_of(L" \t");if(first==std::wstring::npos){profileError=L"Please enter your name to keep your scores.";InvalidateRect(hwnd,nullptr,FALSE);return;}
 const auto last=playerName.find_last_not_of(L" \t");playerName=playerName.substr(first,last-first+1);
 loadProfile();newGame();roundStart();
}
int takeNextFlag(){
 if(deckIndex>=flagDeck.size()){std::shuffle(flagDeck.begin(),flagDeck.end(),rng);deckIndex=0;}
 return flagDeck[deckIndex++];
}
void finishRun(bool gameOver=false);
void updateClock(){
 if(screen!=Screen::Playing||selectedMode!=GameMode::Timed)return;
 const ULONGLONG elapsed=(GetTickCount64()-gameStart)/1000;
 const int previous=remainingSeconds;remainingSeconds=std::max(0,timeLimitSeconds-static_cast<int>(elapsed));
 if(remainingSeconds==0)finishRun();
 else if(previous!=remainingSeconds){RECT timerArea{810,42,950,86},progressArea{52,91,1068,103};InvalidateRect(hwnd,&timerArea,FALSE);InvalidateRect(hwnd,&progressArea,FALSE);}
}
void advanceRound(){
 if(screen!=Screen::Playing||!locked)return;
 if(selectedMode==GameMode::Standard){
  if(static_cast<size_t>(roundNo+1)>=ROUND_COUNT){screen=Screen::Result;InvalidateRect(hwnd,nullptr,FALSE);return;}
  ++roundNo;
 }else{
  rounds.push_back(takeNextFlag());
  roundNo=static_cast<int>(rounds.size()-1);
 }
 roundStart();
}
void finishRun(bool gameOver){if(screen==Screen::Playing&&!rounds.empty()){KillTimer(hwnd,1);if(selectedMode==GameMode::Timed)KillTimer(hwnd,2);lastScore=score;highScore=std::max(highScore,score);saveProfile();screen=gameOver?Screen::GameOver:Screen::Result;locked=false;InvalidateRect(hwnd,nullptr,FALSE);}}
void drawBg(Graphics& g){ brush(g,BG,0,0,W,H); for(int i=0;i<8;i++){SolidBrush b(Color(23,123,142,91));g.FillEllipse(&b,870+i*25,80+i*17,300,300);} }
void drawModeCard(Graphics& g,const Box& box,GameMode mode,const wchar_t* title,const wchar_t* description){
 const bool isSelected=selectedMode==mode;
 const bool isHovered=hoverMode==(mode==GameMode::Standard?0:mode==GameMode::Endless?1:2);
 Color fill=isSelected?Color(255, 55, 77, 64):(isHovered?Color(255, 57, 64, 55):PANEL);
 rounded(g,fill,box.x,box.y,box.w,box.h,16);
 outline(g,isSelected?ACCENT:(isHovered?Color(255,123,151,126):LINE),box.x,box.y,box.w,box.h,16,isSelected?2.0f:1.0f);
 rounded(g,isSelected?ACCENT:Color(255, 68, 76, 66),box.x+20,box.y+31,54,54,16);
 txt(g,mode==GameMode::Standard?L"10":mode==GameMode::Endless?L"∞":L"◷",box.x+20,box.y+31,54,54,25,isSelected?PANEL:ACCENT,true,1);
 txt(g,title,box.x+91,box.y+22,box.w-190,30,20,UI_TEXT,true);
 txt(g,description,box.x+91,box.y+57,box.w-112,44,14,MUTED);
 if(isSelected){rounded(g,ACCENT,box.x+box.w-105,box.y+22,85,24,12);txt(g,L"CHOSEN",box.x+box.w-105,box.y+22,85,24,10,PANEL,true,1);}
}
void drawHome(Graphics& g){
 txt(g,L"FIELD NOTES     /     A WORLD ATLAS GAME",70,34,500,25,12,ACCENT,true);
 rounded(g,PANEL,changePasswordBtn.x,changePasswordBtn.y,changePasswordBtn.w,changePasswordBtn.h,11);outline(g,LINE,changePasswordBtn.x,changePasswordBtn.y,changePasswordBtn.w,changePasswordBtn.h,11,1);txt(g,L"CHANGE PASSWORD",changePasswordBtn.x,changePasswordBtn.y,changePasswordBtn.w,changePasswordBtn.h,10,ACCENT,true,1);
 rounded(g,PANEL,leaderboardBtn.x,leaderboardBtn.y,leaderboardBtn.w,leaderboardBtn.h,11);outline(g,LINE,leaderboardBtn.x,leaderboardBtn.y,leaderboardBtn.w,leaderboardBtn.h,11,1);txt(g,L"LEADERBOARDS",leaderboardBtn.x,leaderboardBtn.y,leaderboardBtn.w,leaderboardBtn.h,11,ACCENT,true,1);
 rounded(g,PANEL,logoutBtn.x,logoutBtn.y,logoutBtn.w,logoutBtn.h,11);outline(g,LINE,logoutBtn.x,logoutBtn.y,logoutBtn.w,logoutBtn.h,11,1);txt(g,L"SIGN OUT",logoutBtn.x,logoutBtn.y,logoutBtn.w,logoutBtn.h,11,MUTED,true,1);
 txt(g,L"Guess the Flag",70,79,770,70,52,UI_TEXT,true);
 txt(g,L"A relaxed world tour, one flag at a time.",74,151,820,32,19,MUTED);
 rounded(g,PANEL,70,209,980,190,22);txt(g,L"WELCOME, EXPLORER",103,229,330,24,12,ACCENT,true);
 txt(g,L"Explore the atlas, build a streak, and see how many flags you know.",103,266,620,58,19,UI_TEXT,true);
 txt(g,L"Fast correct answers earn more points. Flags won't repeat until you've seen them all.",103,336,650,34,14,MUTED);
 txt(g,L"SIGNED IN AS",790,235,235,23,11,ACCENT,true);
 txt(g,playerName,790,264,235,34,20,UI_TEXT,true);
 txt(g,L"HIGH SCORE  "+std::to_wstring(highScore),790,311,235,22,12,ACCENT,true);
 txt(g,L"LAST SCORE  "+std::to_wstring(lastScore),790,337,235,22,12,MUTED,true);
 txt(g,L"195 COUNTRIES",790,365,235,20,11,MUTED,true);
 txt(g,L"CHOOSE HOW TO PLAY",50,425,400,28,14,UI_TEXT,true);
 drawModeCard(g,standardModeBox,GameMode::Standard,L"Standard",L"10 flags · no clock");
 drawModeCard(g,endlessModeBox,GameMode::Endless,L"Endless",L"Play at your own pace");
 drawModeCard(g,timedModeBox,GameMode::Timed,L"Timed",L"Score before time runs out");
 if(selectedMode==GameMode::Timed){
  txt(g,L"TIME LIMIT",325,596,74,30,10,MUTED,true,2);
  const std::array<std::pair<Box,int>,3> times{{{timer5Box,300},{timer2Box,120},{timer1Box,60}}};
  for(const auto& item:times){const bool on=timeLimitSeconds==item.second;rounded(g,on?ACCENT:PANEL,item.first.x,item.first.y,item.first.w,item.first.h,10);outline(g,on?ACCENT:LINE,item.first.x,item.first.y,item.first.w,item.first.h,10,1);txt(g,item.second==300?L"5 MIN":item.second==120?L"2 MIN":L"1 MIN",item.first.x,item.first.y,item.first.w,item.first.h,11,on?PANEL:ACCENT,true,1);}
 }
 const wchar_t* startLabel=selectedMode==GameMode::Standard?L"START STANDARD     →":selectedMode==GameMode::Endless?L"START ENDLESS     →":L"START TIMED     →";
 rounded(g,ACCENT,startBtn.x,startBtn.y,startBtn.w,startBtn.h,16);txt(g,startLabel,startBtn.x,startBtn.y,startBtn.w,startBtn.h,16,PANEL,true,1);
 if(!profileError.empty())txt(g,profileError,65,700,990,25,11,RED,true,1);
 else txt(g,L"CHOOSE A MODE  ·  PRESS ENTER TO BEGIN  ·  SCORES ARE SAVED PER ACCOUNT",0,700,W,22,11,MUTED,true,1);
}
void drawEye(Graphics& g,const Box& box,bool visible){
 rounded(g,PANEL2,box.x,box.y,box.w,box.h,8);Pen pen(ACCENT,1.6f);g.DrawEllipse(&pen,box.x+11,box.y+14,20,13);
 SolidBrush pupil(ACCENT);g.FillEllipse(&pupil,box.x+18,box.y+18,6,6);
 if(!visible){Pen slash(MUTED,1.4f);g.DrawLine(&slash,box.x+12,box.y+31,box.x+30,box.y+10);}
}
void drawCaret(Graphics& g,const std::wstring& value,const Box& box,float size,bool focused,int reserved=56,size_t index=std::wstring::npos){
 if(!focused||!caretVisible)return;
 FontFamily family(L"Segoe UI");Font font(&family,size,FontStyleRegular,UnitPixel);RectF measure;
 if(index==std::wstring::npos||index>value.size())index=value.size();const std::wstring before=value.substr(0,index);
 if(!before.empty())g.MeasureString(before.c_str(),-1,&font,PointF(0,0),&measure);else measure.Width=0;
 const REAL x=std::min(static_cast<REAL>(box.x+14)+measure.Width+2.0f,static_cast<REAL>(box.x+box.w-reserved));
 Pen pen(ACCENT,1.4f);g.DrawLine(&pen,x,static_cast<REAL>(box.y+12),x,static_cast<REAL>(box.y+box.h-12));
}
void startCaretBlink(){caretVisible=true;if(hwnd)SetTimer(hwnd,3,500,nullptr);}
void stopCaretBlink(){if(hwnd)KillTimer(hwnd,3);caretVisible=false;}
void copyRecoveryCode(){
 if(recoveryCode.empty()||!OpenClipboard(hwnd))return;
 EmptyClipboard();const SIZE_T bytes=(recoveryCode.size()+1)*sizeof(wchar_t);HGLOBAL memory=GlobalAlloc(GMEM_MOVEABLE,bytes);
 if(memory){void* data=GlobalLock(memory);if(data){std::memcpy(data,recoveryCode.c_str(),bytes);GlobalUnlock(memory);if(!SetClipboardData(CF_UNICODETEXT,memory))GlobalFree(memory);}else GlobalFree(memory);}
 CloseClipboard();
}
void drawAccount(Graphics& g){
 txt(g,L"FIELD NOTES     /     PLAYER ACCOUNTS",70,42,570,24,12,ACCENT,true);
 txt(g,L"Your own atlas journal",70,88,850,64,42,UI_TEXT,true);
 txt(g,L"Sign in to keep a separate scorecard and record for each mode.",73,154,850,30,17,MUTED);
 rounded(g,PANEL,170,207,780,505,22);
 txt(g,L"PLAYER SIGN IN",220,230,400,28,14,ACCENT,true);
 txt(g,L"Choose an account or create one. Passwords are stored as salted PBKDF2-SHA-256 hashes.",220,264,680,28,13,MUTED);
 txt(g,L"ACCOUNT NAME",260,282,600,20,11,ACCENT,true);
 rounded(g,userFocused?Color(255,65,83,69):PANEL2,userNameBox.x,userNameBox.y,userNameBox.w,userNameBox.h,11);
 outline(g,userFocused?ACCENT:LINE,userNameBox.x,userNameBox.y,userNameBox.w,userNameBox.h,11,userFocused?2.0f:1.0f);
 txt(g,userInput.empty()?L"3–20 characters: letters, numbers, _ or -":userInput,userNameBox.x+14,userNameBox.y,userNameBox.w-28,userNameBox.h,14,userInput.empty()?MUTED:UI_TEXT,!userInput.empty());drawCaret(g,userInput,userNameBox,14,userFocused,56,userCaret);
 txt(g,L"PASSWORD",260,365,600,20,11,ACCENT,true);
 rounded(g,passwordFocused?Color(255,65,83,69):PANEL2,passwordBox.x,passwordBox.y,passwordBox.w,passwordBox.h,11);
 outline(g,passwordFocused?ACCENT:LINE,passwordBox.x,passwordBox.y,passwordBox.w,passwordBox.h,11,passwordFocused?2.0f:1.0f);
 const std::wstring mask(passwordInput.size(),L'•'),passwordShown=showPassword?passwordInput:mask;
 txt(g,passwordInput.empty()?L"At least 8 characters":passwordShown,passwordBox.x+14,passwordBox.y,passwordBox.w-64,passwordBox.h,15,passwordInput.empty()?MUTED:UI_TEXT);
 drawCaret(g,passwordShown,passwordBox,15,passwordFocused,56,passwordCaret);drawEye(g,loginEyeBox,showPassword);
 rounded(g,ACCENT,loginBtn.x,loginBtn.y,loginBtn.w,loginBtn.h,13);txt(g,L"SIGN IN",loginBtn.x,loginBtn.y,loginBtn.w,loginBtn.h,14,PANEL,true,1);
 rounded(g,PANEL2,createAccountBtn.x,createAccountBtn.y,createAccountBtn.w,createAccountBtn.h,13);outline(g,LINE,createAccountBtn.x,createAccountBtn.y,createAccountBtn.w,createAccountBtn.h,13,1);txt(g,L"CREATE ACCOUNT",createAccountBtn.x,createAccountBtn.y,createAccountBtn.w,createAccountBtn.h,14,ACCENT,true,1);
 rounded(g,PANEL2,forgotPasswordBtn.x,forgotPasswordBtn.y,forgotPasswordBtn.w,forgotPasswordBtn.h,8);txt(g,L"FORGOT PASSWORD?",forgotPasswordBtn.x,forgotPasswordBtn.y,forgotPasswordBtn.w,forgotPasswordBtn.h,10,ACCENT,true,1);
 if(!authError.empty())txt(g,authError,560,526,300,27,11,RED,true,1);
 const size_t shown=std::min(accounts.size()-std::min(accountOffset,accounts.size()),accountRowBoxes.size());
 std::wstring range=accounts.empty()?L"NO SAVED ACCOUNTS":L"SAVED ACCOUNTS  ·  "+std::to_wstring(accountOffset+1)+L"–"+std::to_wstring(accountOffset+shown)+L" OF "+std::to_wstring(accounts.size());
 txt(g,range+L"  ·  SELECT ONE, THEN ENTER ITS PASSWORD",260,554,600,20,10,MUTED,true);
 if(accountOffset>0){rounded(g,PANEL2,accountPrev.x,accountPrev.y,accountPrev.w,accountPrev.h,7);txt(g,L"‹",accountPrev.x,accountPrev.y,accountPrev.w,accountPrev.h,14,ACCENT,true,1);}
 if(accountOffset+shown<accounts.size()){rounded(g,PANEL2,accountNext.x,accountNext.y,accountNext.w,accountNext.h,7);txt(g,L"›",accountNext.x,accountNext.y,accountNext.w,accountNext.h,14,ACCENT,true,1);}
 for(size_t i=0;i<shown;i++){const Box& row=accountRowBoxes[i];const std::wstring& account=accounts[accountOffset+i];const bool active=canonicalName(account)==canonicalName(userInput);rounded(g,active?Color(255,55,77,64):PANEL2,row.x,row.y,row.w,row.h,8);outline(g,active?ACCENT:LINE,row.x,row.y,row.w,row.h,8,active?1.5f:1.0f);txt(g,account,row.x+12,row.y,row.w-24,row.h,12,UI_TEXT,true);}
 if(accounts.empty())txt(g,L"No saved accounts on this device yet.",260,589,600,24,12,MUTED);
}
void drawForgotPassword(Graphics& g){
 txt(g,L"FIELD NOTES     /     ACCOUNT RECOVERY",70,42,600,24,12,ACCENT,true);
 txt(g,L"Reset your password",70,88,850,64,42,UI_TEXT,true);
 txt(g,L"Use the recovery key saved when this account was created.",73,154,850,30,17,MUTED);
 rounded(g,PANEL,170,207,780,505,22);txt(g,L"ACCOUNT DETAILS",220,230,400,28,14,ACCENT,true);
 txt(g,L"ACCOUNT NAME",260,276,600,20,11,ACCENT,true);
 rounded(g,recoveryUserFocused?Color(255,65,83,69):PANEL2,recoveryUserBox.x,recoveryUserBox.y,recoveryUserBox.w,recoveryUserBox.h,10);outline(g,recoveryUserFocused?ACCENT:LINE,recoveryUserBox.x,recoveryUserBox.y,recoveryUserBox.w,recoveryUserBox.h,10,recoveryUserFocused?2.0f:1.0f);
 txt(g,userInput.empty()?L"Your saved account name":userInput,recoveryUserBox.x+14,recoveryUserBox.y,recoveryUserBox.w-28,recoveryUserBox.h,14,userInput.empty()?MUTED:UI_TEXT);drawCaret(g,userInput,recoveryUserBox,14,recoveryUserFocused,56,userCaret);
 txt(g,L"RECOVERY KEY",260,345,600,20,11,ACCENT,true);
 rounded(g,recoveryKeyFocused?Color(255,65,83,69):PANEL2,recoveryKeyBox.x,recoveryKeyBox.y,recoveryKeyBox.w,recoveryKeyBox.h,10);outline(g,recoveryKeyFocused?ACCENT:LINE,recoveryKeyBox.x,recoveryKeyBox.y,recoveryKeyBox.w,recoveryKeyBox.h,10,recoveryKeyFocused?2.0f:1.0f);
 const std::wstring recoveryShown=showPassword?recoveryInput:std::wstring(recoveryInput.size(),L'•');
 txt(g,recoveryInput.empty()?L"32-character recovery key":recoveryShown,recoveryKeyBox.x+14,recoveryKeyBox.y,recoveryKeyBox.w-64,recoveryKeyBox.h,14,recoveryInput.empty()?MUTED:UI_TEXT);drawCaret(g,recoveryShown,recoveryKeyBox,14,recoveryKeyFocused,56,recoveryCaret);drawEye(g,recoveryKeyEyeBox,showPassword);
 txt(g,L"NEW PASSWORD",260,414,548,20,11,ACCENT,true);
 rounded(g,newPasswordFocused?Color(255,65,83,69):PANEL2,recoveryNewPasswordBox.x,recoveryNewPasswordBox.y,recoveryNewPasswordBox.w,recoveryNewPasswordBox.h,10);outline(g,newPasswordFocused?ACCENT:LINE,recoveryNewPasswordBox.x,recoveryNewPasswordBox.y,recoveryNewPasswordBox.w,recoveryNewPasswordBox.h,10,newPasswordFocused?2.0f:1.0f);
 const std::wstring newShown=showPassword?newPasswordInput:std::wstring(newPasswordInput.size(),L'•');
 txt(g,newPasswordInput.empty()?L"At least 8 characters":newShown,recoveryNewPasswordBox.x+14,recoveryNewPasswordBox.y,recoveryNewPasswordBox.w-60,recoveryNewPasswordBox.h,14,newPasswordInput.empty()?MUTED:UI_TEXT);drawCaret(g,newShown,recoveryNewPasswordBox,14,newPasswordFocused,56,newPasswordCaret);drawEye(g,recoveryNewEyeBox,showPassword);
 txt(g,L"CONFIRM NEW PASSWORD",260,483,548,20,11,ACCENT,true);
 rounded(g,confirmPasswordFocused?Color(255,65,83,69):PANEL2,recoveryConfirmPasswordBox.x,recoveryConfirmPasswordBox.y,recoveryConfirmPasswordBox.w,recoveryConfirmPasswordBox.h,10);outline(g,confirmPasswordFocused?ACCENT:LINE,recoveryConfirmPasswordBox.x,recoveryConfirmPasswordBox.y,recoveryConfirmPasswordBox.w,recoveryConfirmPasswordBox.h,10,confirmPasswordFocused?2.0f:1.0f);
 const std::wstring confirmShown=showPassword?confirmPasswordInput:std::wstring(confirmPasswordInput.size(),L'•');
 txt(g,confirmPasswordInput.empty()?L"Type the new password again":confirmShown,recoveryConfirmPasswordBox.x+14,recoveryConfirmPasswordBox.y,recoveryConfirmPasswordBox.w-60,recoveryConfirmPasswordBox.h,14,confirmPasswordInput.empty()?MUTED:UI_TEXT);drawCaret(g,confirmShown,recoveryConfirmPasswordBox,14,confirmPasswordFocused,56,confirmPasswordCaret);drawEye(g,recoveryConfirmEyeBox,showPassword);
 if(!authError.empty())txt(g,authError,260,557,600,24,11,RED,true,1);
 rounded(g,PANEL2,forgotBackBtn.x,forgotBackBtn.y,forgotBackBtn.w,forgotBackBtn.h,12);outline(g,LINE,forgotBackBtn.x,forgotBackBtn.y,forgotBackBtn.w,forgotBackBtn.h,12,1);txt(g,L"BACK",forgotBackBtn.x,forgotBackBtn.y,forgotBackBtn.w,forgotBackBtn.h,12,ACCENT,true,1);
 rounded(g,ACCENT,resetPasswordBtn.x,resetPasswordBtn.y,resetPasswordBtn.w,resetPasswordBtn.h,12);txt(g,L"RESET PASSWORD",resetPasswordBtn.x,resetPasswordBtn.y,resetPasswordBtn.w,resetPasswordBtn.h,13,PANEL,true,1);
}
void drawRecovery(Graphics& g){
 txt(g,L"FIELD NOTES     /     SAVE YOUR ACCOUNT KEY",70,42,720,24,12,ACCENT,true);
 txt(g,L"Keep this recovery key",70,88,900,64,42,UI_TEXT,true);
 rounded(g,PANEL,170,207,780,505,22);
 txt(g,L"Your account is ready, "+playerName+L".",220,242,680,35,22,UI_TEXT,true);
 txt(g,L"Save this key somewhere private. It is shown only once and lets you reset your password.",220,300,680,52,15,MUTED);
 rounded(g,PANEL2,250,385,620,70,14);outline(g,ACCENT,250,385,620,70,14,1.5f);
 txt(g,recoveryCode,270,385,580,70,23,ACCENT,true,1);
 rounded(g,PANEL2,copyRecoveryBtn.x,copyRecoveryBtn.y,copyRecoveryBtn.w,copyRecoveryBtn.h,12);outline(g,LINE,copyRecoveryBtn.x,copyRecoveryBtn.y,copyRecoveryBtn.w,copyRecoveryBtn.h,12,1);txt(g,L"COPY KEY",copyRecoveryBtn.x,copyRecoveryBtn.y,copyRecoveryBtn.w,copyRecoveryBtn.h,13,ACCENT,true,1);
 rounded(g,ACCENT,recoveryContinueBtn.x,recoveryContinueBtn.y,recoveryContinueBtn.w,recoveryContinueBtn.h,12);txt(g,L"I'VE SAVED IT",recoveryContinueBtn.x,recoveryContinueBtn.y,recoveryContinueBtn.w,recoveryContinueBtn.h,13,PANEL,true,1);
}
void drawLeaderboard(Graphics& g){
 txt(g,L"FIELD NOTES     /     LOCAL SCOREBOARD",70,38,600,24,12,ACCENT,true);
 txt(g,L"WORLD LEADERBOARD",70,80,800,56,38,UI_TEXT,true);
 txt(g,L"Top scores from player accounts saved on this device.",73,143,780,26,15,MUTED);
 const std::array<std::wstring,5> labels{{L"STANDARD",L"ENDLESS",L"5 MIN",L"2 MIN",L"1 MIN"}};
 for(size_t i=0;i<labels.size();i++){const bool active=(i==0&&selectedMode==GameMode::Standard)||(i==1&&selectedMode==GameMode::Endless)||(i==2&&selectedMode==GameMode::Timed&&timeLimitSeconds==300)||(i==3&&selectedMode==GameMode::Timed&&timeLimitSeconds==120)||(i==4&&selectedMode==GameMode::Timed&&timeLimitSeconds==60);const Box& b=leaderboardModeBoxes[i];rounded(g,active?ACCENT:PANEL,b.x,b.y,b.w,b.h,10);outline(g,active?ACCENT:LINE,b.x,b.y,b.w,b.h,10,1);txt(g,labels[i],b.x,b.y,b.w,b.h,12,active?PANEL:ACCENT,true,1);}
 rounded(g,PANEL,70,255,980,370,18);txt(g,L"PLAYER",110,274,520,28,12,MUTED,true);txt(g,L"PERSONAL BEST",720,274,280,28,12,MUTED,true,2);
 for(size_t i=0;i<10;i++){const int y=313+static_cast<int>(i)*29;if(i<boardRows.size()){const bool me=canonicalName(boardRows[i].user)==canonicalName(playerName);if(me)rounded(g,Color(255,55,77,64),95,y,930,27,7);txt(g,L"#"+std::to_wstring(i+1)+L"     "+boardRows[i].user,110,y,650,27,14,me?ACCENT:UI_TEXT,true);txt(g,std::to_wstring(boardRows[i].score),720,y,280,27,14,me?ACCENT:UI_TEXT,true,2);}else{txt(g,L"-",110,y,650,27,13,MUTED);txt(g,L"-",720,y,280,27,13,MUTED,true,2);}}
 if(selectedMode==GameMode::Endless)txt(g,L"ENDLESS WORLD BOARD  ·  LOCAL PLAYERS ONLY FOR NOW",95,606,930,24,10,ACCENT,true,1);
 rounded(g,PANEL2,backBtn.x,backBtn.y,backBtn.w,backBtn.h,12);outline(g,LINE,backBtn.x,backBtn.y,backBtn.w,backBtn.h,12,1);txt(g,(previousScreen==Screen::Result||previousScreen==Screen::GameOver)?L"BACK TO RESULTS":L"BACK TO MAIN MENU",backBtn.x,backBtn.y,backBtn.w,backBtn.h,12,ACCENT,true,1);
}
void drawGame(Graphics& g){
 txt(g,L"FIELD NOTES     /     GEOGRAPHY DESK",55,25,440,22,11,ACCENT,true);
 txt(g,selectedMode==GameMode::Standard?L"STANDARD TOUR":selectedMode==GameMode::Endless?L"ENDLESS TOUR":L"TIMED CHALLENGE",55,57,350,26,19,UI_TEXT,true);
 wstring prog=L"FLAG  ";if(roundNo+1<10)prog+=L"0";prog+=std::to_wstring(roundNo+1);
 if(selectedMode==GameMode::Standard)prog+=L"  /  "+std::to_wstring(ROUND_COUNT);
 txt(g,prog,selectedMode==GameMode::Standard?720:635,53,selectedMode==GameMode::Standard?220:140,24,13,MUTED,true,2);
 if(selectedMode==GameMode::Timed){const int minutes=remainingSeconds/60,seconds=remainingSeconds%60;wstring clock=(minutes<10?L"0":L"")+std::to_wstring(minutes)+L":"+(seconds<10?L"0":L"")+std::to_wstring(seconds);txt(g,clock,820,48,110,32,20,RED,true,1);}
 if(selectedMode==GameMode::Timed){rounded(g,hoverFinish?Color(255, 62, 79, 66):PANEL,finishBtn.x,finishBtn.y,finishBtn.w,finishBtn.h,12);outline(g,LINE,finishBtn.x,finishBtn.y,finishBtn.w,finishBtn.h,12,1);txt(g,L"FINISH RUN",finishBtn.x,finishBtn.y,finishBtn.w,finishBtn.h,11,ACCENT,true,1);}
 rounded(g,LINE,55,94,1010,5,3);
 if(selectedMode==GameMode::Standard)rounded(g,ACCENT,55,94,1010*(roundNo+1)/static_cast<int>(ROUND_COUNT),5,3);
 else if(selectedMode==GameMode::Timed)rounded(g,RED,55,94,1010*remainingSeconds/timeLimitSeconds,5,3);else rounded(g,ACCENT,55,94,1010,5,3);
 rounded(g,PANEL,55,122,1010,324,20); txt(g,L"Can you name this country?",88,146,720,22,13,MUTED,true);
 flag(g,rounds[roundNo],370,188,380,218);
 txt(g,L"CHOOSE A COUNTRY",55,462,400,22,12,MUTED,true);
 for(size_t i=0;i<ANSWER_COUNT;i++){Box b=optionBoxes[i]; Color fill=PANEL;Color edge=LINE; if(locked&&choices[i]==rounds[roundNo]){fill=Color(255, 43, 82, 61);edge=GREEN;}else if(locked&&selected==static_cast<int>(i)){fill=Color(255, 91, 53, 47);edge=RED;}else if(hover==static_cast<int>(i)){fill=Color(255, 59, 72, 60);edge=ACCENT;}rounded(g,fill,b.x,b.y,b.w,b.h,13);outline(g,edge,b.x,b.y,b.w,b.h,13,1.3f);wstring label=std::to_wstring(i+1)+L"     "+countries[static_cast<size_t>(choices[i])].name;txt(g,label,b.x+18,b.y,b.w-36,b.h,17,UI_TEXT,true);}
 if(locked){const bool good=selected>=0&&static_cast<size_t>(selected)<choices.size()&&choices[static_cast<size_t>(selected)]==rounds[static_cast<size_t>(roundNo)]; txt(g,good?answerFeedback:(wstring(L"Almost - this is the flag of ")+countries[static_cast<size_t>(rounds[static_cast<size_t>(roundNo)])].name+L"."),55,625,650,28,14,good?GREEN:RED,true);rounded(g,ACCENT,nextBtn.x,nextBtn.y,nextBtn.w,nextBtn.h,13);const wchar_t* nextText=(selectedMode==GameMode::Standard&&static_cast<size_t>(roundNo+1)==ROUND_COUNT)?L"SEE YOUR RESULTS   →":L"NEXT FLAG   →";txt(g,nextText,nextBtn.x,nextBtn.y,nextBtn.w,nextBtn.h,14,PANEL,true,1);}
 else txt(g,L"Click an answer, or press 1, 2, 3 or 4.",55,625,500,25,12,MUTED,true);
 txt(g,L"SCORE  "+std::to_wstring(score),790,615,270,24,13,ACCENT,true,2);
 txt(g,L"BEST  "+std::to_wstring(highScore)+L"     STREAK  "+std::to_wstring(streak)+L"×",790,642,270,22,11,MUTED,true,2);
 if(selectedMode==GameMode::Endless)txt(g,L"WRONG ANSWERS  "+std::to_wstring(mistakes)+L" / 5",55,660,500,20,11,RED,true);
}
void drawResult(Graphics& g){
 const size_t total=rounds.size();
 const size_t shown=std::min(ROUND_COUNT,total);
 txt(g,L"FIELD NOTES     /     GEOGRAPHY DESK",70,40,500,25,12,ACCENT,true);
 txt(g,screen==Screen::GameOver?L"GAME OVER":selectedMode==GameMode::Standard?L"A lovely trip around the atlas.":L"Your atlas, your pace.",70,101,920,65,39,UI_TEXT,true);
 const std::wstring resultSubtitle=screen==Screen::GameOver?L"Five wrong answers brought this Endless run to a close.":L"PLAYER  "+playerName+L"     ·     "+(selectedMode==GameMode::Standard?wstring(L"Ten-flag scorecard."):selectedMode==GameMode::Endless?wstring(L"Endless atlas run."):wstring(L"Timed challenge complete."));
 txt(g,resultSubtitle,73,176,900,30,16,MUTED);
 rounded(g,PANEL,70,238,980,252,22);txt(g,std::to_wstring(score),105,262,250,110,76,ACCENT,true);
 txt(g,L"POINTS EARNED",111,374,240,25,13,MUTED,true);
 txt(g,L"CORRECT ANSWERS  "+std::to_wstring(correctCount)+L" / "+std::to_wstring(total),390,395,590,27,15,ACCENT,true);
 txt(g,L"PERSONAL BEST  "+std::to_wstring(highScore)+L"     BEST STREAK  "+std::to_wstring(maxStreak),390,427,590,27,14,MUTED,true);
 std::wstring message;
 if(selectedMode==GameMode::Standard){
  message=correctCount==static_cast<int>(ROUND_COUNT)?L"Every flag, right on the mark. Wonderful!":correctCount>=7?L"A lovely score. Your flag knowledge is growing.":correctCount>=4?L"A good start. There are so many more to discover.":L"Thanks for playing. You’ll recognize more next time.";
 }else{
  const double accuracy=total?static_cast<double>(correctCount)/static_cast<double>(total):0.0;
  message=accuracy>=0.8?L"You recognized "+std::to_wstring(correctCount)+L" of "+std::to_wstring(total)+L" flags. Wonderful tour!":accuracy>=0.5?L"You recognized "+std::to_wstring(correctCount)+L" of "+std::to_wstring(total)+L" flags. Lovely exploring!":L"You recognized "+std::to_wstring(correctCount)+L" of "+std::to_wstring(total)+L" flags. Keep wandering!";
 }
 if(screen==Screen::GameOver)message=L"Your tour is over for now. Check the leaderboard or start another run.";
 txt(g,message,390,314,590,64,18,UI_TEXT,true);
 const size_t first=total-shown;
 std::wstring summaryLabel=selectedMode==GameMode::Standard?L"YOUR TEN FLAGS":L"YOUR LAST "+std::to_wstring(shown)+L" FLAGS";
 txt(g,summaryLabel,70,516,360,24,12,ACCENT,true);
 for(size_t i=0;i<shown;i++){const size_t round=first+i;int x=70+static_cast<int>(i%4)*248,y=550+static_cast<int>(i/4)*53;rounded(g,PANEL2,x,y,228,42,10);txt(g,L"0"+std::to_wstring(round+1)+L"   "+countries[static_cast<size_t>(rounds[round])].name,x+12,y,206,42,13,UI_TEXT,true);}
 rounded(g,PANEL2,70,650,220,56,13);outline(g,LINE,70,650,220,56,13,1);txt(g,L"CHOOSE A MODE",70,650,220,56,13,ACCENT,true,1);
 rounded(g,PANEL2,resultLeaderboardBtn.x,resultLeaderboardBtn.y,resultLeaderboardBtn.w,resultLeaderboardBtn.h,13);outline(g,LINE,resultLeaderboardBtn.x,resultLeaderboardBtn.y,resultLeaderboardBtn.w,resultLeaderboardBtn.h,13,1);txt(g,L"LEADERBOARD",resultLeaderboardBtn.x,resultLeaderboardBtn.y,resultLeaderboardBtn.w,resultLeaderboardBtn.h,13,ACCENT,true,1);
 rounded(g,ACCENT,720,650,330,56,13);txt(g,L"PLAY AGAIN     →",720,650,330,56,15,PANEL,true,1);
}
void drawFrame(Graphics& g){
 g.SetSmoothingMode(SmoothingModeAntiAlias);
 g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
 drawBg(g);
 if(screen==Screen::Account)drawAccount(g);else if(screen==Screen::Home)drawHome(g);else if(screen==Screen::Playing)drawGame(g);else if(screen==Screen::Result||screen==Screen::GameOver)drawResult(g);else if(screen==Screen::ForgotPassword)drawForgotPassword(g);else if(screen==Screen::Recovery)drawRecovery(g);else drawLeaderboard(g);
}
void paint(){
 PAINTSTRUCT ps;HDC target=BeginPaint(hwnd,&ps);
 RECT client{};GetClientRect(hwnd,&client);
 const int width=client.right-client.left,height=client.bottom-client.top;
 HDC buffer=CreateCompatibleDC(target);
 HBITMAP bitmap=width>0&&height>0?CreateCompatibleBitmap(target,width,height):nullptr;
 if(buffer&&bitmap){
  HGDIOBJ previous=SelectObject(buffer,bitmap);
  {Graphics graphics(buffer);drawFrame(graphics);}
  BitBlt(target,0,0,width,height,buffer,0,0,SRCCOPY);
  SelectObject(buffer,previous);DeleteObject(bitmap);DeleteDC(buffer);
 }else{
  if(bitmap)DeleteObject(bitmap);if(buffer)DeleteDC(buffer);
  Graphics graphics(target);drawFrame(graphics);
 }
 EndPaint(hwnd,&ps);
}
void invalidateOption(HWND h,int index){
 if(index<0||static_cast<size_t>(index)>=optionBoxes.size())return;
 const Box& box=optionBoxes[static_cast<size_t>(index)];
 RECT region{box.x-3,box.y-3,box.x+box.w+3,box.y+box.h+3};
 InvalidateRect(h,&region,FALSE);
}
void invalidateBox(HWND h,const Box& box){RECT region{box.x-3,box.y-3,box.x+box.w+3,box.y+box.h+3};InvalidateRect(h,&region,FALSE);}
void choose(int i){
 if(selectedMode==GameMode::Timed)updateClock();
 if(screen!=Screen::Playing||locked||i<0||static_cast<size_t>(i)>=ANSWER_COUNT||static_cast<size_t>(i)>=choices.size())return;
 selected=i;locked=true;
 if(choices[static_cast<size_t>(i)]==rounds[static_cast<size_t>(roundNo)]){
  ++correctCount;++streak;maxStreak=std::max(maxStreak,streak);const ULONGLONG elapsed=GetTickCount64()-questionStart;
  const int speedBonus=std::max(0,100-static_cast<int>(std::min<ULONGLONG>(elapsed,5000)*100/5000));
  const double multiplier=std::min(3.0,1.0+0.25*static_cast<double>(streak-1));
  lastAward=static_cast<int>((100+speedBonus)*multiplier);score+=lastAward;
  const int multiplierQuarters=std::min(12,4+streak-1);const wchar_t* fraction=multiplierQuarters%4==0?L"00":multiplierQuarters%4==1?L"25":multiplierQuarters%4==2?L"50":L"75";
  answerFeedback=L"Correct! +"+std::to_wstring(lastAward)+L" points  ·  streak x"+std::to_wstring(multiplierQuarters/4)+L"."+fraction;
 }else{streak=0;lastAward=0;answerFeedback=L"Not quite - your streak starts fresh next round.";if(selectedMode==GameMode::Endless)++mistakes;}
 if(selectedMode==GameMode::Endless&&mistakes>=5){finishRun(true);return;}
 lastScore=score;highScore=std::max(highScore,score);saveProfile();
 InvalidateRect(hwnd,nullptr,FALSE);SetTimer(hwnd,1,450,FALSE);
}
LRESULT CALLBACK proc(HWND h,UINT m,WPARAM wp,LPARAM lp){
 switch(m){
 case WM_PAINT: paint(); return 0;
 case WM_ERASEBKGND: return 1;
 case WM_MOUSEMOVE:{
  if(!trackingMouse){TRACKMOUSEEVENT event{sizeof(event),TME_LEAVE,h,0};trackingMouse=TrackMouseEvent(&event)!=FALSE;}
  int x=GET_X_LPARAM(lp),y=GET_Y_LPARAM(lp),old=hover,oldMode=hoverMode;bool oldFinish=hoverFinish;
  hover=-1;hoverMode=-1;hoverFinish=false;
  if(screen==Screen::Playing&&!locked)for(size_t i=0;i<ANSWER_COUNT;i++)if(optionBoxes[i].has(x,y))hover=static_cast<int>(i);
  if(screen==Screen::Home){if(standardModeBox.has(x,y))hoverMode=0;else if(endlessModeBox.has(x,y))hoverMode=1;else if(timedModeBox.has(x,y))hoverMode=2;}
  if(screen==Screen::Playing&&selectedMode==GameMode::Timed&&finishBtn.has(x,y))hoverFinish=true;
  if(old!=hover){invalidateOption(h,old);invalidateOption(h,hover);}
  if(oldMode!=hoverMode){if(oldMode==0)invalidateBox(h,standardModeBox);else if(oldMode==1)invalidateBox(h,endlessModeBox);else if(oldMode==2)invalidateBox(h,timedModeBox);if(hoverMode==0)invalidateBox(h,standardModeBox);else if(hoverMode==1)invalidateBox(h,endlessModeBox);else if(hoverMode==2)invalidateBox(h,timedModeBox);}
  if(oldFinish!=hoverFinish)invalidateBox(h,finishBtn);
  return 0;
 }
 case WM_MOUSELEAVE:{int old=hover,oldMode=hoverMode;bool oldFinish=hoverFinish;hover=-1;hoverMode=-1;hoverFinish=false;trackingMouse=false;invalidateOption(h,old);if(oldMode==0)invalidateBox(h,standardModeBox);else if(oldMode==1)invalidateBox(h,endlessModeBox);else if(oldMode==2)invalidateBox(h,timedModeBox);if(oldFinish)invalidateBox(h,finishBtn);return 0;}
 case WM_LBUTTONUP:{
  int x=GET_X_LPARAM(lp),y=GET_Y_LPARAM(lp);
  if(screen==Screen::Account){
   if(loginEyeBox.has(x,y)){showPassword=!showPassword;InvalidateRect(h,nullptr,FALSE);return 0;}
   if(forgotPasswordBtn.has(x,y)){resetFromLogin=true;screen=Screen::ForgotPassword;recoveryUserFocused=true;recoveryKeyFocused=newPasswordFocused=confirmPasswordFocused=false;clearRecoveryFields();showPassword=false;authError.clear();startCaretBlink();InvalidateRect(h,nullptr,FALSE);return 0;}
   userFocused=userNameBox.has(x,y);passwordFocused=passwordBox.has(x,y);
   if(userFocused)passwordFocused=false;
   if(passwordFocused)userFocused=false;
   if(userFocused||passwordFocused)startCaretBlink();else stopCaretBlink();
   if(loginBtn.has(x,y)){stopCaretBlink();signIn();InvalidateRect(h,nullptr,FALSE);return 0;}
   if(createAccountBtn.has(x,y)){stopCaretBlink();createAccount();InvalidateRect(h,nullptr,FALSE);return 0;}
   if(accountPrev.has(x,y)&&accountOffset>=accountRowBoxes.size())accountOffset-=accountRowBoxes.size();
   else if(accountNext.has(x,y)&&accountOffset+accountRowBoxes.size()<accounts.size())accountOffset+=accountRowBoxes.size();
   else for(size_t i=0;i<std::min(accounts.size()-std::min(accountOffset,accounts.size()),accountRowBoxes.size());i++)if(accountRowBoxes[i].has(x,y)){userInput=accounts[accountOffset+i];clearPassword();userFocused=false;passwordFocused=true;authError.clear();startCaretBlink();break;}
   InvalidateRect(h,nullptr,FALSE);return 0;
  }
  if(screen==Screen::Recovery){
   if(copyRecoveryBtn.has(x,y))copyRecoveryCode();
   else if(recoveryContinueBtn.has(x,y)){screen=Screen::Home;showPassword=false;stopCaretBlink();if(!recoveryCode.empty())SecureZeroMemory(recoveryCode.data(),recoveryCode.size()*sizeof(wchar_t));recoveryCode.clear();}
   InvalidateRect(h,nullptr,FALSE);return 0;
  }
  if(screen==Screen::ForgotPassword){
   if(recoveryKeyEyeBox.has(x,y)||recoveryNewEyeBox.has(x,y)||recoveryConfirmEyeBox.has(x,y)){showPassword=!showPassword;InvalidateRect(h,nullptr,FALSE);return 0;}
   recoveryUserFocused=recoveryUserBox.has(x,y);recoveryKeyFocused=recoveryKeyBox.has(x,y);newPasswordFocused=recoveryNewPasswordBox.has(x,y);confirmPasswordFocused=recoveryConfirmPasswordBox.has(x,y);
   const bool fieldFocused=recoveryUserFocused||recoveryKeyFocused||newPasswordFocused||confirmPasswordFocused;
   if(fieldFocused)startCaretBlink();else stopCaretBlink();
   if(resetPasswordBtn.has(x,y)){stopCaretBlink();resetAccountPassword();InvalidateRect(h,nullptr,FALSE);return 0;}
   if(forgotBackBtn.has(x,y)){screen=resetFromLogin?Screen::Account:Screen::Home;clearRecoveryFields();authError.clear();showPassword=false;stopCaretBlink();InvalidateRect(h,nullptr,FALSE);return 0;}
   InvalidateRect(h,nullptr,FALSE);return 0;
  }
  if(screen==Screen::Home){
   if(changePasswordBtn.has(x,y)){resetFromLogin=false;userInput=playerName;clearRecoveryFields();screen=Screen::ForgotPassword;recoveryUserFocused=false;recoveryKeyFocused=true;newPasswordFocused=confirmPasswordFocused=false;showPassword=false;authError.clear();startCaretBlink();InvalidateRect(h,nullptr,FALSE);return 0;}
   if(leaderboardBtn.has(x,y)){previousScreen=Screen::Home;previousMode=selectedMode;previousTimeLimit=timeLimitSeconds;loadLeaderboard();screen=Screen::Leaderboard;InvalidateRect(h,nullptr,FALSE);return 0;}
   if(logoutBtn.has(x,y)){screen=Screen::Account;userInput=playerName;clearPassword();passwordFocused=true;userFocused=false;authError.clear();loadAccounts();startCaretBlink();InvalidateRect(h,nullptr,FALSE);return 0;}
   bool statsChanged=false;
   if(standardModeBox.has(x,y)){selectedMode=GameMode::Standard;statsChanged=true;}
   else if(endlessModeBox.has(x,y)){selectedMode=GameMode::Endless;statsChanged=true;}
   else if(timedModeBox.has(x,y)){selectedMode=GameMode::Timed;statsChanged=true;}
   else if(selectedMode==GameMode::Timed&&timer5Box.has(x,y)){timeLimitSeconds=300;statsChanged=true;}
   else if(selectedMode==GameMode::Timed&&timer2Box.has(x,y)){timeLimitSeconds=120;statsChanged=true;}
   else if(selectedMode==GameMode::Timed&&timer1Box.has(x,y)){timeLimitSeconds=60;statsChanged=true;}
   else if(startBtn.has(x,y)){beginGame();return 0;}
   if(statsChanged)loadProfile();
   InvalidateRect(h,nullptr,FALSE);return 0;
  }
  else if(screen==Screen::Playing){
   if(selectedMode==GameMode::Timed&&finishBtn.has(x,y))finishRun();
   else{for(size_t i=0;i<ANSWER_COUNT;i++)if(optionBoxes[i].has(x,y))choose(static_cast<int>(i));if(locked&&nextBtn.has(x,y))advanceRound();}
  }else if((screen==Screen::Result||screen==Screen::GameOver)&&homeBtn.has(x,y)){screen=Screen::Home;loadProfile();InvalidateRect(h,nullptr,FALSE);}
  else if((screen==Screen::Result||screen==Screen::GameOver)&&resultLeaderboardBtn.has(x,y)){previousScreen=screen;previousMode=selectedMode;previousTimeLimit=timeLimitSeconds;loadLeaderboard();screen=Screen::Leaderboard;InvalidateRect(h,nullptr,FALSE);return 0;}
  else if((screen==Screen::Result||screen==Screen::GameOver)&&playAgainBtn.has(x,y)){beginGame();}
  else if(screen==Screen::Leaderboard&&backBtn.has(x,y)){screen=previousScreen;selectedMode=previousMode;timeLimitSeconds=previousTimeLimit;loadProfile();InvalidateRect(h,nullptr,FALSE);return 0;}
  else if(screen==Screen::Leaderboard){
   for(size_t i=0;i<leaderboardModeBoxes.size();i++)if(leaderboardModeBoxes[i].has(x,y)){
    if(i==0)selectedMode=GameMode::Standard;
    else if(i==1)selectedMode=GameMode::Endless;
    else{selectedMode=GameMode::Timed;timeLimitSeconds=i==2?300:i==3?120:60;}
    loadLeaderboard();break;
   }
  }
  InvalidateRect(h,nullptr,FALSE);return 0;
 }
 case WM_CHAR:
  if(screen==Screen::Account&&(userFocused||passwordFocused)){
   std::wstring& target=userFocused?userInput:passwordInput;size_t& caret=userFocused?userCaret:passwordCaret;const size_t limit=userFocused?20:128;
   if(wp==L'\b'){if(caret>0){target.erase(caret-1,1);--caret;}}
   else if(wp>=32&&wp<127&&target.size()<limit){if(passwordFocused){target.insert(caret,1,static_cast<wchar_t>(wp));++caret;}else if((wp>=L'a'&&wp<=L'z')||(wp>=L'A'&&wp<=L'Z')||(wp>=L'0'&&wp<=L'9')||wp==L'_'||wp==L'-'){target.insert(caret,1,static_cast<wchar_t>(wp));++caret;}}
   else if(wp==L'\x7f'&&caret<target.size())target.erase(caret,1);
   authError.clear();caretVisible=true;startCaretBlink();InvalidateRect(h,nullptr,FALSE);return 0;
  }
  if(screen==Screen::ForgotPassword&&(recoveryUserFocused||recoveryKeyFocused||newPasswordFocused||confirmPasswordFocused)){
   std::wstring* target=recoveryUserFocused?&userInput:recoveryKeyFocused?&recoveryInput:newPasswordFocused?&newPasswordInput:&confirmPasswordInput;
   size_t& caret=recoveryUserFocused?userCaret:recoveryKeyFocused?recoveryCaret:newPasswordFocused?newPasswordCaret:confirmPasswordCaret;
   const size_t limit=recoveryUserFocused?20:recoveryKeyFocused?40:128;
   if(wp==L'\b'){if(caret>0){target->erase(caret-1,1);--caret;}}
   else if(wp>=32&&wp<127&&target->size()<limit){
    if(recoveryUserFocused){if((wp>=L'a'&&wp<=L'z')||(wp>=L'A'&&wp<=L'Z')||(wp>=L'0'&&wp<=L'9')||wp==L'_'||wp==L'-'){target->insert(caret,1,static_cast<wchar_t>(wp));++caret;}}
    else{target->insert(caret,1,static_cast<wchar_t>(wp));++caret;}
   }
   else if(wp==L'\x7f'&&caret<target->size())target->erase(caret,1);
   authError.clear();caretVisible=true;startCaretBlink();InvalidateRect(h,nullptr,FALSE);return 0;
  }
  break;
 case WM_KEYDOWN:
  if((screen==Screen::Account&&(userFocused||passwordFocused))||(screen==Screen::ForgotPassword&&(recoveryUserFocused||recoveryKeyFocused||newPasswordFocused||confirmPasswordFocused))){
   std::wstring* target=nullptr;size_t* caret=nullptr;
   if(screen==Screen::Account){target=userFocused?&userInput:&passwordInput;caret=userFocused?&userCaret:&passwordCaret;}
   else if(recoveryUserFocused){target=&userInput;caret=&userCaret;}
   else if(recoveryKeyFocused){target=&recoveryInput;caret=&recoveryCaret;}
   else if(newPasswordFocused){target=&newPasswordInput;caret=&newPasswordCaret;}
   else{target=&confirmPasswordInput;caret=&confirmPasswordCaret;}
   if((GetKeyState(VK_CONTROL)&0x8000)&&(wp=='V'||wp=='v')&&OpenClipboard(h)){
    HANDLE clip=GetClipboardData(CF_UNICODETEXT);
    if(clip){const wchar_t* data=static_cast<const wchar_t*>(GlobalLock(clip));if(data){std::wstring pasted(data);GlobalUnlock(clip);const size_t limit=(screen==Screen::Account?(userFocused?20:128):(recoveryUserFocused?20:recoveryKeyFocused?40:128));
      for(wchar_t ch:pasted){if(target->size()>=limit)break;if(screen==Screen::ForgotPassword&&recoveryKeyFocused){if(ch==L' '||ch==L'-')continue;if(!((ch>=L'a'&&ch<=L'z')||(ch>=L'A'&&ch<=L'Z')||(ch>=L'0'&&ch<=L'9')))continue;ch=static_cast<wchar_t>(towupper(ch));}
       else if((screen==Screen::Account&&userFocused)||(screen==Screen::ForgotPassword&&recoveryUserFocused)){if(!((ch>=L'a'&&ch<=L'z')||(ch>=L'A'&&ch<=L'Z')||(ch>=L'0'&&ch<=L'9')||ch==L'_'||ch==L'-'))continue;}
       else if(ch<32||ch>126)continue;target->insert((*caret)++,1,ch);}
      authError.clear();startCaretBlink();InvalidateRect(h,nullptr,FALSE);}}
    CloseClipboard();return 0;
   }
   if(wp==VK_LEFT){if(*caret>0)--*caret;InvalidateRect(h,nullptr,FALSE);return 0;}
   if(wp==VK_RIGHT){if(*caret<target->size())++*caret;InvalidateRect(h,nullptr,FALSE);return 0;}
   if(wp==VK_HOME){*caret=0;InvalidateRect(h,nullptr,FALSE);return 0;}
   if(wp==VK_END){*caret=target->size();InvalidateRect(h,nullptr,FALSE);return 0;}
  }
  if(screen==Screen::Account&&wp==VK_TAB){if(userFocused){userFocused=false;passwordFocused=true;}else{userFocused=true;passwordFocused=false;}startCaretBlink();InvalidateRect(h,nullptr,FALSE);}
  else if(screen==Screen::Account&&wp==VK_RETURN){if(userFocused){userFocused=false;passwordFocused=true;startCaretBlink();InvalidateRect(h,nullptr,FALSE);}else{stopCaretBlink();signIn();}}
  else if(screen==Screen::Account&&wp==VK_ESCAPE){stopCaretBlink();passwordFocused=false;userFocused=true;InvalidateRect(h,nullptr,FALSE);}
  else if(screen==Screen::ForgotPassword&&wp==VK_TAB){
   if(recoveryUserFocused){recoveryUserFocused=false;recoveryKeyFocused=true;}
   else if(recoveryKeyFocused){recoveryKeyFocused=false;newPasswordFocused=true;}
   else if(newPasswordFocused){newPasswordFocused=false;confirmPasswordFocused=true;}
   else{recoveryUserFocused=true;confirmPasswordFocused=false;}
   startCaretBlink();InvalidateRect(h,nullptr,FALSE);
  }
  else if(screen==Screen::ForgotPassword&&wp==VK_RETURN){
   if(confirmPasswordFocused){stopCaretBlink();resetAccountPassword();}
   else if(recoveryUserFocused){recoveryUserFocused=false;recoveryKeyFocused=true;startCaretBlink();}
   else if(recoveryKeyFocused){recoveryKeyFocused=false;newPasswordFocused=true;startCaretBlink();}
   else if(newPasswordFocused){newPasswordFocused=false;confirmPasswordFocused=true;startCaretBlink();}
   InvalidateRect(h,nullptr,FALSE);
  }
  else if(screen==Screen::ForgotPassword&&wp==VK_ESCAPE){screen=resetFromLogin?Screen::Account:Screen::Home;clearRecoveryFields();authError.clear();showPassword=false;stopCaretBlink();InvalidateRect(h,nullptr,FALSE);}
  else if(screen==Screen::Recovery&&wp==VK_RETURN){screen=Screen::Home;stopCaretBlink();if(!recoveryCode.empty())SecureZeroMemory(recoveryCode.data(),recoveryCode.size()*sizeof(wchar_t));recoveryCode.clear();InvalidateRect(h,nullptr,FALSE);}
  else if(screen==Screen::Home&&(wp==VK_LEFT||wp==L'1')){selectedMode=GameMode::Standard;loadProfile();InvalidateRect(h,nullptr,FALSE);}
  else if(screen==Screen::Home&&(wp==VK_RIGHT||wp==L'2')){selectedMode=GameMode::Endless;loadProfile();InvalidateRect(h,nullptr,FALSE);}
  else if(screen==Screen::Home&&wp==L'3'){selectedMode=GameMode::Timed;loadProfile();InvalidateRect(h,nullptr,FALSE);}
  else if(screen==Screen::Home&&(wp==VK_RETURN||wp==VK_SPACE)){beginGame();}
  else if(screen==Screen::Playing){
   if(wp>=L'1'&&wp<=L'4')choose(static_cast<int>(wp-L'1'));
   else if(locked&&(wp==VK_RETURN||wp==VK_SPACE))advanceRound();
   else if(selectedMode==GameMode::Timed&&wp==VK_ESCAPE)finishRun();
  }else if((screen==Screen::Result||screen==Screen::GameOver)&&(wp==VK_RETURN||wp==VK_SPACE)){beginGame();}
  else if((screen==Screen::Result||screen==Screen::GameOver)&&wp==L'L'){previousScreen=screen;previousMode=selectedMode;previousTimeLimit=timeLimitSeconds;loadLeaderboard();screen=Screen::Leaderboard;}
  else if((screen==Screen::Result||screen==Screen::GameOver)&&wp==VK_ESCAPE){screen=Screen::Home;loadProfile();InvalidateRect(h,nullptr,FALSE);}
  else if(screen==Screen::Leaderboard&&wp==VK_ESCAPE){screen=previousScreen;selectedMode=previousMode;timeLimitSeconds=previousTimeLimit;loadProfile();InvalidateRect(h,nullptr,FALSE);}
  InvalidateRect(h,nullptr,FALSE);return 0;
 case WM_TIMER:
  if(wp==1){KillTimer(h,1);InvalidateRect(h,nullptr,FALSE);}
  else if(wp==2)updateClock();
  else if(wp==3){
   caretVisible=!caretVisible;const Box* box=nullptr;
   if(screen==Screen::Account){if(userFocused)box=&userNameBox;else if(passwordFocused)box=&passwordBox;}
   else if(screen==Screen::ForgotPassword){if(recoveryUserFocused)box=&recoveryUserBox;else if(recoveryKeyFocused)box=&recoveryKeyBox;else if(newPasswordFocused)box=&recoveryNewPasswordBox;else if(confirmPasswordFocused)box=&recoveryConfirmPasswordBox;}
   if(box){RECT area{box->x-2,box->y-2,box->x+box->w+2,box->y+box->h+2};InvalidateRect(h,&area,FALSE);}else KillTimer(h,3);
  }
  return 0;
 case WM_DESTROY: clearPassword();clearRecoveryFields();if(!recoveryCode.empty())SecureZeroMemory(recoveryCode.data(),recoveryCode.size()*sizeof(wchar_t));recoveryCode.clear();countries.clear();GdiplusShutdown(gdip);gdip=0;PostQuitMessage(0);return 0;
 }
 return DefWindowProcW(h,m,wp,lp);
}
}
int WINAPI wWinMain(HINSTANCE inst,HINSTANCE,LPWSTR,int show){
 GdiplusStartupInput startupInput;
 if(GdiplusStartup(&gdip,&startupInput,nullptr)!=Ok){
  MessageBoxW(nullptr,L"GDI+ could not be initialized. The game cannot display its flag images.",L"Guess the Flag",MB_OK|MB_ICONERROR);
  return 1;
 }
 if(!loadCountries()){
  MessageBoxW(nullptr,loadError.c_str(),L"Guess the Flag - Asset Error",MB_OK|MB_ICONERROR);
  countries.clear();GdiplusShutdown(gdip);gdip=0;return 1;
 }
 if(!loadWarnings.empty())MessageBoxW(nullptr,loadWarnings.c_str(),L"Guess the Flag - Some Assets Skipped",MB_OK|MB_ICONWARNING);
 loadAccounts();
 WNDCLASSW wc{};wc.lpfnWndProc=proc;wc.hInstance=inst;wc.lpszClassName=L"GuessTheFlagWindow";wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);wc.hbrBackground=nullptr;
 wc.hIcon=static_cast<HICON>(LoadImageW(inst,MAKEINTRESOURCEW(IDI_APP),IMAGE_ICON,32,32,LR_DEFAULTCOLOR));
 smallAppIcon=static_cast<HICON>(LoadImageW(inst,MAKEINTRESOURCEW(IDI_APP),IMAGE_ICON,16,16,LR_DEFAULTCOLOR));
 if(!wc.hIcon)wc.hIcon=LoadIconW(nullptr,IDI_APPLICATION);
 if(!smallAppIcon)smallAppIcon=wc.hIcon;
 if(!RegisterClassW(&wc)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS){
  MessageBoxW(nullptr,L"The game window could not be registered.",L"Guess the Flag",MB_OK|MB_ICONERROR);
  countries.clear();GdiplusShutdown(gdip);gdip=0;return 1;
 }
 RECT r{0,0,W,H};AdjustWindowRect(&r,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,FALSE);
 hwnd=CreateWindowExW(0,wc.lpszClassName,L"Guess the Flag",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,nullptr,nullptr,inst,nullptr);
 if(!hwnd){
  MessageBoxW(nullptr,L"The game window could not be created.",L"Guess the Flag",MB_OK|MB_ICONERROR);
  countries.clear();GdiplusShutdown(gdip);gdip=0;return 1;
 }
 SendMessageW(hwnd,WM_SETICON,ICON_SMALL,reinterpret_cast<LPARAM>(smallAppIcon));
 ShowWindow(hwnd,show);UpdateWindow(hwnd);
 MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}
 return static_cast<int>(msg.wParam);
}
