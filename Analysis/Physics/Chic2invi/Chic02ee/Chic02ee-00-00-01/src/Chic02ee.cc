// -*- C++ -*- //
//
// Description: chic02ee -> e+ e- 
//
// Original Author:  Amit pathak <amit@ihep.ac.cn>
//         Created:  [2018-07-25 Mon 16:30] 
//         Inspired by SHI Xin's code 
//         Helped by maoqiang
//


//
// system include files
//


#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/DeclareFactoryEntries.h"
#include "GaudiKernel/LoadFactoryEntries.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/Bootstrap.h"

#include "EvtRecEvent/EvtRecEvent.h"
#include "EvtRecEvent/EvtRecTrack.h"

#include "EventModel/EventHeader.h"
#include "EventModel/EventModel.h"
#include "EventModel/Event.h"
#include "TrigEvent/TrigEvent.h"
#include "TrigEvent/TrigData.h"

#include "CLHEP/Vector/ThreeVector.h"
#include "VertexFit/IVertexDbSvc.h"
#include "VertexFit/Helix.h"
#include "VertexFit/WTrackParameter.h"
#include "VertexFit/VertexFit.h"
#include "DstEvent/TofHitStatus.h"


#include <TFile.h>
#include <TH1.h>
#include <TTree.h>
//
// class declaration //
//

class Chic02ee : public Algorithm {
  
public:
  Chic02ee(const std::string&, ISvcLocator*);
  ~Chic02ee(); 
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:


double m_vr0cut, m_vz0cut;
double m_ecms;
double m_total_number_of_charged_max;
double m_cha_costheta_cut;
double m_lepton_momentum_min;
double m_lepton_momentum_max;
double m_dilepton_mass_min; 
double m_dilepton_mass_max;

//output file
std::string m_output_filename;
bool m_isMonteCarlo; 
bool m_eventrate;
TFile* m_fout; 


//define Histograms
TH1F* h_evtflw;


//define Trees
TTree* m_tree;

//
//common info
//
int m_run;
int m_event;

//
//charged tracks
//
int m_ncharged;
int m_nGoodChargedTrack;
int m_nlptrk;
int m_nlmtrk;
double m_trklp_p;
double m_trklp_px;
double m_trklp_py; 
double m_trklp_pz; 
double m_trklp_theta; 
double m_trklp_phi; 
double m_trklp_eraw;

// lepton info
double m_lp_px;
double m_lp_py;
double m_lp_pz;

double m_lm_px;
double m_lm_py;
double m_lm_pz;

// fitted lepton info
int m_chic02elel_flag;
int m_chic02mumu_flag;


// vertex
double m_vr0;
double m_vz0;


// MC truth info
//
int m_evttag;

//
//chic02ee
//
int m_ntrk;


//
//functions
//
void book_histogram();
void book_tree();
void clearVariables();
bool buildChic02ee();
void saveLeptonInfo(RecMdcKalTrack *,
		      RecMdcKalTrack *);
int selectChargedTracks(SmartDataPtr<EvtRecEvent>,
			  SmartDataPtr<EvtRecTrackCol>,
       	std::vector<int> &,
			  std::vector<int> &,
			  std::vector<int> &);
int selectLeptonPlusLeptonMinus(SmartDataPtr<EvtRecTrackCol>,
				  std::vector<int>,
				  std::vector<int>);
bool hasGoodLpLmVertex(RecMdcKalTrack *,
			  RecMdcKalTrack *,
			  int,
			  bool);

bool passVertexSelection(CLHEP::Hep3Vector,
			  RecMdcKalTrack*);

CLHEP::Hep3Vector getOrigin();

};

//
//module declare 
//
DECLARE_ALGORITHM_FACTORY( Chic02ee )
DECLARE_FACTORY_ENTRIES( Chic02ee ) {
DECLARE_ALGORITHM(Chic02ee);
}
LOAD_FACTORY_ENTRIES( Chic02ee )
//
//constants
//
const double ELECTRON_MASS = 0.000511;
const double MUON_MASS = 0.105658;
const int Electron_Hypothesis_Fit=1;
const int Muon_Hypothesis_Fit=2;

//
//member functions
//


Chic02ee::Chic02ee(const std::string& name, ISvcLocator* pSvcLocator) :
Algorithm(name, pSvcLocator),
m_tree(0)
{
declareProperty("OutputFileName",m_output_filename);
declareProperty("IsMonteCarlo", m_isMonteCarlo);
declareProperty("Ecms", m_ecms = 3.686);
declareProperty("Vr0cut", m_vr0cut=1.0);
declareProperty("Vz0cut", m_vz0cut=10.0);
declareProperty("IsMonteCarlo",m_isMonteCarlo);
declareProperty("TotalNumberOfChargedMax",m_total_number_of_charged_max = 50);
declareProperty("ChaCosthetaCut", m_cha_costheta_cut = 0.93);
declareProperty("LeptonMomentumMax", m_lepton_momentum_min=0.6); 
declareProperty("LeptonMomentumMin", m_lepton_momentum_max=2.5);
declareProperty("DileptonMassMin", m_dilepton_mass_min=2.5); 
declareProperty("DileptonMassMax", m_dilepton_mass_max=3.5); 
}

StatusCode Chic02ee::initialize(){ MsgStream log(msgSvc(), name());
log << MSG::INFO << ">>>>>>> in initialize()" << endmsg;

StatusCode status;
  
m_fout = new TFile(m_output_filename.c_str(), "RECREATE");
m_fout->cd(); 

book_histogram(); 
book_tree(); 

log << MSG::INFO << "successfully return from initialize()" <<endmsg; 

return StatusCode::SUCCESS;
}


StatusCode Chic02ee::execute() {
MsgStream log(msgSvc(), name());
log << MSG::INFO << "in execute()" << endreq;
  
// clear variables 
clearVariables();
h_evtflw->Fill(0); //raw
SmartDataPtr<Event::EventHeader> eventHeader(eventSvc(),"/Event/EventHeader");
if (!eventHeader) return StatusCode::FAILURE;

       
m_run = eventHeader->runNumber();
m_event = eventHeader->eventNumber();

if (buildChic02ee()) {
m_tree->Fill();// only fill tree for the selected events 
}
return StatusCode::SUCCESS; 
}
StatusCode Chic02ee::finalize() {
MsgStream log(msgSvc(), name());
log << MSG::INFO << "in finalize()" << endmsg;

m_fout->cd();
m_tree->Write();
h_evtflw->Write();
m_fout->Close();

return StatusCode::SUCCESS;
}

Chic02ee::~Chic02ee() {
}

void Chic02ee::book_histogram() {

h_evtflw = new TH1F("hevtflw","eventflow",13,0,13);
if (!h_evtflw) return;
h_evtflw->GetXaxis()->SetBinLabel(1, "raw");
h_evtflw->GetXaxis()->SetBinLabel(2, "N_{Good}=4");
h_evtflw->GetXaxis()->SetBinLabel(9, "Lepton P<2.0GeV/c");
}

void Chic02ee::book_tree(){
m_tree=new TTree("tree", "Chic02ee");
if (!m_tree) return;

// common info
m_tree->Branch("run",&m_run,"run/I");
m_tree->Branch("event",&m_event,"event/I");

// charged tracks
m_tree->Branch("nchargedTrack",&m_ncharged,"nchargedTrack/I");
m_tree->Branch("nGoodChargedTrack",&m_nGoodChargedTrack, "nGoodChargedTrack/I");
m_tree->Branch("nlptrk", &m_nlptrk, "nlptrk/I");
m_tree->Branch("nlmtrk", &m_nlmtrk, "nlmtrk/I");

m_tree->Branch("trklp_p", &m_trklp_p, "trklp_p/D");
m_tree->Branch("trklp_px", &m_trklp_px, "trklp_px/D");
m_tree->Branch("trklp_py", &m_trklp_py, "trklp_py/D"); 
m_tree->Branch("trklp_pz", &m_trklp_pz, "trklp_pz/D"); 
m_tree->Branch("trklp_theta", &m_trklp_theta, "trklp_theta/D"); 
m_tree->Branch("trklp_phi", &m_trklp_phi, "trklp_phi/D"); 
m_tree->Branch("trklp_eraw", &m_trklp_eraw, "trklp_eraw/D"); 

//vertex
  m_tree->Branch("vr0", &m_vr0, "vr0/D");
  m_tree->Branch("vz0", &m_vz0, "vz0/D");

}

void Chic02ee::clearVariables(){
m_run=0;
m_event=0;
m_ncharged=-1;
m_nGoodChargedTrack=-1;
m_nlptrk=-1;
m_nlmtrk=-1;
}
bool Chic02ee::buildChic02ee() {

SmartDataPtr<EvtRecEvent>evtRecEvent(eventSvc(),"/Event/EvtRec/EvtRecEvent");
	if(!evtRecEvent) return false;
SmartDataPtr<EvtRecTrackCol> evtRecTrkCol(eventSvc(), "/Event/EvtRec/EvtRecTrackCol");
	if(!evtRecTrkCol) return false;

std::vector<int> iChargedGood;
std::vector<int> iPGood, iMGood;
selectChargedTracks(evtRecEvent, evtRecTrkCol,
            iPGood, iMGood,
            iChargedGood);

//selectChargedTracks(evtRecEvent, evtRecTrkCol, iPGood, iMGood, iChargedGood);

if ( (m_nlptrk != 1) || (m_nlmtrk != 1) ) return false;

h_evtflw->Fill(1); // N_LeptonP=1, N_LeptonM=1

if(selectLeptonPlusLeptonMinus(evtRecTrkCol, iPGood, iMGood) != 1) return false; 

return true;

}

CLHEP::Hep3Vector Chic02ee::getOrigin() {
  CLHEP::Hep3Vector xorigin(0,0,0);
  IVertexDbSvc*  vtxsvc;
  Gaudi::svcLocator()->service("VertexDbSvc", vtxsvc);
  if(vtxsvc->isVertexValid()){
    double *dbv = vtxsvc->PrimaryVertex(); 
    xorigin.setX(dbv[0]);
    xorigin.setY(dbv[1]);
    xorigin.setZ(dbv[2]);
  }
  return xorigin; 
}

bool Chic02ee::passVertexSelection(CLHEP::Hep3Vector xorigin,
				    RecMdcKalTrack* mdcTrk ) {
  HepVector a = mdcTrk->helix();
  HepSymMatrix Ea = mdcTrk->err();
  HepPoint3D point0(0.,0.,0.);
  VFHelix helixip(point0,a,Ea);
  HepPoint3D IP(xorigin[0],xorigin[1],xorigin[2]);
  helixip.pivot(IP);
  HepVector vecipa = helixip.a();
  
  m_vz0 = vecipa[3];
  m_vr0 = vecipa[0];
  
  if(fabs(m_vz0) >= m_vz0cut) return false;
  if(fabs(m_vr0) >= m_vr0cut) return false;
  
  return true;
}

int Chic02ee::selectChargedTracks(SmartDataPtr<EvtRecEvent> evtRecEvent,
				SmartDataPtr<EvtRecTrackCol> evtRecTrkCol,
				std::vector<int> & iChargedGood,
        std::vector<int> & iPGood,
				std::vector<int> & iMGood) {

CLHEP::Hep3Vector xorigin = getOrigin();

std::vector<int> iGood;
iGood.clear();
iPGood.clear();
iMGood.clear();
iChargedGood.clear();
  

// loop through charged tracks 
for(int i = 0; i < evtRecEvent->totalCharged(); i++){
    
// get mdcTrk 
EvtRecTrackIterator itTrk=evtRecTrkCol->begin() + i;

// Good Kalman Track 
if(!(*itTrk)->isMdcKalTrackValid()) continue;
	
if(!(*itTrk)->isMdcTrackValid()) continue; 
RecMdcKalTrack* mdcTrk = (*itTrk)->mdcKalTrack();
	
// Good Vertex 
if (!passVertexSelection(xorigin, mdcTrk)) continue;
	
// Polar angle cut
if(fabs(cos(mdcTrk->theta())) > m_cha_costheta_cut) continue;
    
iChargedGood.push_back((*itTrk)->trackId());
iGood.push_back((*itTrk)->trackId());
if(mdcTrk->charge()>0) iPGood.push_back((*itTrk)->trackId());
if(mdcTrk->charge()<0) iMGood.push_back((*itTrk)->trackId());

} // end charged tracks

m_nGoodChargedTrack = iChargedGood.size();
m_nlptrk = iPGood.size();
m_nlmtrk = iMGood.size(); 

if (m_nlptrk > 0 && m_nlmtrk > 0) {
EvtRecTrackIterator itTrk_lp = evtRecTrkCol->begin() + iPGood[0];
EvtRecTrackIterator itTrk_lm = evtRecTrkCol->begin() + iMGood[0];
}

return iGood.size();
}


int Chic02ee::selectLeptonPlusLeptonMinus(SmartDataPtr<EvtRecTrackCol> evtRecTrkCol,
					   std::vector<int> iPGood,
					   std::vector<int> iMGood) {
int nlplm = 0;
bool evtflw_filled = false;
  
for(unsigned int i1 = 0; i1 < iPGood.size(); i1++) {
EvtRecTrackIterator itTrk_p = evtRecTrkCol->begin() + iPGood[i1];
RecMdcTrack* mdcTrk_p = (*itTrk_p)->mdcTrack();
if (mdcTrk_p->charge() < 0) continue; // only positive charged tracks

for(unsigned int i2 = 0; i2 < iMGood.size(); i2++) {
EvtRecTrackIterator itTrk_m = evtRecTrkCol->begin() + iMGood[i2];
RecMdcTrack* mdcTrk_m = (*itTrk_m)->mdcTrack();
if (mdcTrk_m->charge() > 0) continue; // only negative charged tracks

// lepton momentum 
if ( ! ( fabs(mdcTrk_p->p()) < m_lepton_momentum_max  &&
fabs(mdcTrk_m->p()) < m_lepton_momentum_max )) continue;

if ( !evtflw_filled ) h_evtflw->Fill(8); //|p| cut 
      
// apply vertex fit
RecMdcKalTrack *lpTrk = (*(evtRecTrkCol->begin()+iPGood[i1]))->mdcKalTrack();
RecMdcKalTrack *lmTrk = (*(evtRecTrkCol->begin()+iMGood[i2]))->mdcKalTrack();
saveLeptonInfo(lpTrk, lmTrk);
int ee_flag=0, mumu_flag=0;
if ( hasGoodLpLmVertex(lpTrk, lmTrk, Electron_Hypothesis_Fit, evtflw_filled) ) ee_flag=1;
if ( hasGoodLpLmVertex(lpTrk, lmTrk, Muon_Hypothesis_Fit, evtflw_filled) ) mumu_flag=1; 
      
m_chic02elel_flag = ee_flag;
m_chic02mumu_flag = mumu_flag;
if(ee_flag==1 || mumu_flag==1) nlplm++;

evtflw_filled = true;
}
} 
return nlplm; 
}
void Chic02ee::saveLeptonInfo(RecMdcKalTrack *lpTrk,
			       RecMdcKalTrack *lmTrk){
m_lp_px = lpTrk->px();
m_lp_py = lpTrk->py();
m_lp_pz = lpTrk->pz();

m_lm_px = lmTrk->px();
m_lm_py = lmTrk->py();
m_lm_pz = lmTrk->pz();

}
bool Chic02ee::hasGoodLpLmVertex(RecMdcKalTrack *lpTrk,
				  RecMdcKalTrack *lmTrk,
				  int particle_flag,
				  bool evtflw_filled) {

  HepLorentzVector pcms(0.011*m_ecms, 0., 0., m_ecms);

  HepLorentzVector p4_vtx_lp, p4_vtx_lm, p4_vtx_lplm;
  WTrackParameter wvlpTrk, wvlmTrk;

  if( particle_flag  == Electron_Hypothesis_Fit ){ 
    lpTrk->setPidType(RecMdcKalTrack::electron);
    wvlpTrk = WTrackParameter(ELECTRON_MASS, lpTrk->getZHelixE(), lpTrk->getZErrorE());
    
    lmTrk->setPidType(RecMdcKalTrack::electron);
    wvlmTrk = WTrackParameter(ELECTRON_MASS, lmTrk->getZHelixE(), lmTrk->getZErrorE());
  }
  if( particle_flag == Muon_Hypothesis_Fit ){
    lpTrk->setPidType(RecMdcKalTrack::muon);
    wvlpTrk = WTrackParameter(MUON_MASS, lpTrk->getZHelixMu(), lpTrk->getZErrorMu());
    
    lmTrk->setPidType(RecMdcKalTrack::muon);
    wvlmTrk = WTrackParameter(MUON_MASS, lmTrk->getZHelixMu(), lmTrk->getZErrorMu());
  }
  
  HepPoint3D vx(0., 0., 0.);
  HepSymMatrix Evx(3, 0);
  
  double bx = 1E+6;
  double by = 1E+6;
  double bz = 1E+6;
  Evx[0][0] = bx*bx;
  Evx[1][1] = by*by;
  Evx[2][2] = bz*bz;
  
  VertexParameter vxpar;
  vxpar.setVx(vx);
  vxpar.setEvx(Evx);
  
  VertexFit* vtxfit = VertexFit::instance();
  vtxfit->init();
  vtxfit->AddTrack(0,  wvlpTrk);
  vtxfit->AddTrack(1,  wvlmTrk);
  vtxfit->AddVertex(0, vxpar,0,1);
  
  if(!vtxfit->Fit(0)) return false;
  
  vtxfit->Swim(0);
  
  WTrackParameter wlp = vtxfit->wtrk(0);
  WTrackParameter wlm = vtxfit->wtrk(1);
  p4_vtx_lp = vtxfit->pfit(0) ;
  p4_vtx_lm = vtxfit->pfit(1) ;
  p4_vtx_lplm = p4_vtx_lp + p4_vtx_lm;

  if( ! ( p4_vtx_lplm.m() >= m_dilepton_mass_min &&
	  p4_vtx_lplm.m() <= m_dilepton_mass_max) ) return false; // 2.5<M_{ll}<3.5
  
  if( particle_flag  == Electron_Hypothesis_Fit ){ 
    if( !evtflw_filled ) h_evtflw->Fill(9); 
  }
  if( particle_flag  == Muon_Hypothesis_Fit ){ 
    if( !evtflw_filled ) h_evtflw->Fill(10); 
  }
  
 // saveLeptonVtxInfo(p4_vtx_lp, p4_vtx_lm, particle_flag); 

  return true;
}