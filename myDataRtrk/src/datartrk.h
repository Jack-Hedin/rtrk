#ifndef DATARTRK_H
#define DATARTRK_H

#include <fun4all/SubsysReco.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TFile.h>

#include <string>
#include <utility>

class PHCompositeNode;
class TriggerAnalyzer;
class TFile;

class datartrk : public SubsysReco
{
 public:

  datartrk(const std::string& recojetnameR04,
           const std::string& outputfilename);

  ~datartrk() override;

  int Init(PHCompositeNode*) override;
  int process_event(PHCompositeNode*) override;
  int End(PHCompositeNode*) override;

  void set_eta_range(float min, float max) { m_etaRange = {min, max}; }
  void set_pt_range(float min, float max)  { m_ptRange  = {min, max}; }
  void set_verbosity(int v)                { m_verbosity = v; }
  void set_embedding_id(int id)            { m_embedding_id = id; }

 private:
  float CalculateDeltaR(float eta1, float phi1, float eta2, float phi2);
  bool  processVertex(PHCompositeNode* topNode);
  bool  processTrigger(PHCompositeNode* topNode);
  int   processRtrkAnalysis(PHCompositeNode* topNode);

  std::string m_recoJetNameR04;
  std::string m_outputFileName;
  std::pair<float, float> m_etaRange;
  std::pair<float, float> m_ptRange;
  int m_verbosity;
  int m_embedding_id;

  int m_total_events;
  int m_total_jets;

  float m_trk_pt_min;
  float m_matching_radius;
  int   m_trk_quality_max;

  //TriggerAnalyzer* trigAna;

  TFile *out{nullptr};
  TH2F *h_rtrk_vs_jet_pt_emcal;
  TH1F *h_nMatchedTracks_emcal;
  TH2F *h_jet_eta_phi;
  TH1F *h_deltaR_emcal_jet_track;
  TH1F *h_deta_emcal_jet_track;
  TH1F *h_dphi_emcal_jet_track;
  TH2F *h_track_eta_phi_emcal;
  TH1F *h_zvtx_all;
  TH1F *h_zvtx_cut;
  TH1F *h_jet_pt;
  TH1F *h_track_pt;
  TH2F *h_sumtrkpt_vs_jetpt;
  TH2F *h_nTracks_vs_sumPtTracks;
};

#endif // DATARTRK_H
