#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

struct County {
  int stnr;
  string fylke;
};

struct Munip {
  int stnr;
  string komm;
};
vector<County> countyList;
vector<Munip> munipList;
int webs[41] = {4780,7010,10380,16610,17150,18700,23420,24890,27500,
		31620,36200,39100,42160,44560,47300,50500,50540,54120,
		59800,60500,69100,71550,75410,75550,80610,82290,85380,
		87110,89350,90450,93140,93700,94500,96400,98550,99370,
		99710,99720,99840,99910,99950};

int pri1s[25] = {4780,10380,18500,18700,24890,27500,36200,39040,44560,
		 50540,54120,60500,69100,70850,72800,82290,90450,93700,
		 97251,98550,99710,99720,99840,99910,99950};

int pri2s[11] = {3190,17150,28380,35860,60990,63420,68860,68863,93140,94500,95350};

int pri3s[155] = {180,700,1130,2540,4440,4460,6020,7010,8140,9580,11500,12320,
		  12550,12680,13160,13420,13670,15730,16610,16740,17000,18950,
		  19710,20301,21680,23160,23420,23500,23550,25110,25830,26900,
		  26990,27450,27470,28800,29720,30420,30650,31620,32060,33890,
		  34130,36560,37230,38140,39100,39690,40880,41110,41670,41770,
		  42160,42920,43010,44080,44610,45870,46510,46610,46910,47260,
		  47300,48120,48330,49800,50070,50300,50500,51530,51800,52290,
		  52535,52860,53101,55290,55700,55820,57420,57710,57770,58070,
		  58900,59110,59610,59680,59800,61180,61770,62270,62480,63705,
		  64330,64550,65310,65940,66730,68340,69150,69380,70150,71000,
		  71850,71990,72060,72580,73500,75220,75410,75550,76330,76450,
		  76530,76750,77230,77550,78800,79600,80101,80102,80610,80700,
		  81680,82410,83550,85380,85450,85891,85892,86500,86740,87110,
		  87640,88200,88690,89350,90490,90800,91380,91760,92350,93301,
		  93900,94280,94680,96310,96400,96800,97350,98090,98400,98790,
		  99370,99760,99765};

bool webStat(int stnr) {
  for ( int i = 0; i < 41; i++ ) {
    if ( stnr == webs[i] ) return true;
  }
  return false;
}

bool pri1Stat(int stnr) {
  for ( int i = 0; i < 25; i++ ) {
    if ( stnr == pri1s[i] ) return true;
  }
  return false;
}

bool pri2Stat(int stnr) {
  for ( int i = 0; i < 11; i++ ) {
    if ( stnr == pri2s[i] ) return true;
  }
  return false;
}

bool pri3Stat(int stnr) {
  for ( int i = 0; i < 155; i++ ) {
    if ( stnr == pri3s[i] ) return true;
  }
  return false;
}



int main() {
  ifstream inf1("/home/knutj/kvhqc/stinfo_fylke");
  ifstream inf2("/home/knutj/kvhqc/stinfo_kommune");
  County cty;
  Munip mnp;
  while ( !inf1.eof() ) {
    char* buf = new char[40];
    inf1.getline(buf,39);    
    if ( inf1.eof() ) break;
    char* cStnr = new char[9];
    strncpy(cStnr,&buf[2],8);
    int stnr = atoi(cStnr);
    char* cFylk = new char[20];
    strncpy(cFylk,&buf[12],19);
    cty.stnr = stnr;
    cty.fylke = string(cFylk);
    if ( cty.fylke.compare("SVALBARD") == 0 || cty.fylke.compare("JAN MAYEN") == 0 )
      cty.fylke = "ISHAVET";
    //    if ( !cty.fylke.empty() )
    if ( cty.stnr >0 && cty.stnr <= 99999 )
      countyList.push_back(cty);
    delete[] buf;
    delete[] cStnr;
    delete[] cFylk;
  }
  while ( !inf2.eof() ) {
    char* buf = new char[40];
    inf2.getline(buf,39);    
    if ( inf2.eof() ) break;
    char* cStnr = new char[9];
    strncpy(cStnr,&buf[2],8);
    int stnr = atoi(cStnr);
    char* cKomm = new char[25];
    strncpy(cKomm,&buf[12],24);
    mnp.stnr = stnr;
    mnp.komm = string(cKomm);
    //    if ( !mnp.komm.empty() )
    if ( mnp.stnr >0 && mnp.stnr <= 99999 )
      munipList.push_back(mnp);
    delete[] buf;
    delete[] cStnr;
    delete[] cKomm;
  }

  vector<Munip>::iterator mit = munipList.begin();
  cout.flags(ios::fixed);
  for ( vector<County>::iterator cit = countyList.begin(); cit != countyList.end(); cit++ ) {
    if ( mit->stnr == cit->stnr ) {
      cout << right << setw(7) << cit->stnr << " " << left << setw(31) << cit->fylke << left << setw(25) << mit->komm;
      if ( webStat(cit->stnr) )
	cout << setw(3) << "WEB";
      if ( pri1Stat(cit->stnr) && webStat(cit->stnr) ) 
	cout << setw(4) << "PRI1";
      else if ( pri1Stat(cit->stnr) )
	cout << setw(7) << "   PRI1";
      if ( pri2Stat(cit->stnr) && webStat(cit->stnr) ) 
	cout << setw(4) << "PRI2";
      else if ( pri2Stat(cit->stnr) )
	cout << setw(7) << "   PRI2";
      if ( pri3Stat(cit->stnr) && webStat(cit->stnr) ) 
	cout << setw(4) << "PRI3";
      else if ( pri3Stat(cit->stnr) )
	cout << setw(7) << "   PRI3";
      cit--;
      mit++;
      cout << endl;
    }
  }
    cout << "  99986" << endl;
    cout << "  99987" << endl;          
    cout << "  99988" << endl;                 
    cout << "  99989" << endl;                
    cout << "  99990" << endl;                
    cout << "  99991" << endl;              
    cout << "  99992" << endl;                
    cout << "  99993" << endl;              
    cout << "  99994" << endl;                   
    cout << "  99995" << endl;             
    cout << "  99996" << endl;                       
    cout << "  99997" << endl;                     
    cout << "  99998" << endl;                         
    cout << "  99999" << endl;
    cout << " 202000 NORDLAND" << endl;
    cout << " 203600 NORDLAND" << endl;
    cout << " 208000 FINNMARK" << endl;
    cout << " 210400 NORDLAND" << endl;
    cout << " 211000 NORDLAND" << endl;
    cout << " 220600 NORD-TRØNDELAG" << endl;
    cout << " 222200 NORD-TRØNDELAG" << endl;
    cout << " 230800 SØR-TRØNDELAG" << endl;
    cout << " 241800 ØSTFOLD" << endl;
    cout << " 250000 ØSTFOLD" << endl;
    cout << " 280700 FINNMARK" << endl;
  inf1.close();
  inf2.close();
  return 0;
}
