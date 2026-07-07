
#include "rtrk.h"

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/PHTFileServer.h>
#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>

#include <jetbase/Jet.h>
#include <jetbase/JetContainer.h>
#include <trackbase_historic/SvtxTrackMap.h>
#include <trackbase_historic/SvtxTrack.h>
#include <trackbase_historic/SvtxTrackState.h>
#include <calobase/RawTowerGeomContainer.h>
#include <globalvertex/GlobalVertex.h>
#include <globalvertex/GlobalVertexMap.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TFile.h>
#include <TVector3.h>

#include <cmath>
#include <iostream>

using namespace std;

//____________________________________________________________________________..
rtrk::rtrk(const std::string& recojetnameR04,
           const std::string& outputfilename)
  : SubsysReco("rtrk")
  , m_recoJetNameR04(recojetnameR04)
  , m_outputFileName(outputfilename)
  , m_etaRange(-0.7, 0.7)
  , m_ptRange(10.0, 100.0)
  , m_verbosity(1)
  , m_embedding_id(1)
  , m_total_events(0)
  , m_total_jets(0)
  , m_trk_pt_min(0.2f)
  , m_matching_radius(0.4f)
  , m_trk_quality_max(200)
{
  //std::cout << "rtrk::rtrk(const std::string &name) Calling ctor" << std::endl;
}

//____________________________________________________________________________..
rtrk::~rtrk()
{
  std::cout << "rtrk::Destructor - Cleaning up" << std::endl;
}

//____________________________________________________________________________..
int rtrk::Init(PHCompositeNode*)
{
  PHTFileServer::get().open(m_outputFileName, "RECREATE");

  h_rtrk_vs_jet_pt_emcal =
    new TH2F("h_rtrk_vs_jet_pt_emcal",
             "r_{trk} vs Jet p_{T} (EMCal);Jet p_{T} [GeV/c];#Sigma p_{T}^{trk}/p_{T}^{jet}",
             100, 10, 100, 100, 0, 1.5);

  h_nMatchedTracks_emcal =
    new TH1F("h_nMatchedTracks_emcal",
             "Matched tracks per jet (EMCal);N;Counts",
             20, 0, 20);

  h_jet_eta_phi =
    new TH2F("h_jet_eta_phi",
             "Jets;#eta;#phi",
             60, -1.5, 1.5, 64, -3.2, 3.2);

  h_deltaR_emcal_jet_track =
    new TH1F("h_deltaR_emcal_jet_track",
             "EMCal #DeltaR(jet,track);#DeltaR;Counts",
             100, 0, 1.0);

  h_deta_emcal_jet_track =
    new TH1F("h_deta_emcal_jet_track",
             "EMCal #Delta#eta;#Delta#eta;Counts",
             100, -2.0, 2.0);

  h_dphi_emcal_jet_track =
    new TH1F("h_dphi_emcal_jet_track",
             "EMCal #Delta#phi;#Delta#phi;Counts",
             100, -M_PI, M_PI);

  h_track_eta_phi_emcal =
    new TH2F("h_track_eta_phi_emcal",
             "Tracks (EMCal proj);#eta;#phi",
             60, -1.5, 1.5, 64, -3.2, 3.2);

  h_zvtx_all =
    new TH1F("h_zvtx_all",
             "z-vertex (all);z [cm];Counts",
             100, -150, 150);

  h_zvtx_cut =
    new TH1F("h_zvtx_cut",
             "z-vertex (|z|<10);z [cm];Counts",
             100, -150, 150);

  h_jet_pt =
    new TH1F("h_jet_pt",
             "Jet p_{T};p_{T} [GeV/c];Counts",
             100, 10, 100);

  h_track_pt =
    new TH1F("h_track_pt",
             "Track p_{T};p_{T} [GeV/c];Counts",
             200, 0.2, 30);

  h_sumtrkpt_vs_jetpt =
    new TH2F("h_sumtrkpt_vs_jetpt",
           "#Sigma p_{T}^{trk} vs Jet p_{T} (EMCal);Jet p_{T} [GeV/c];#Sigma p_{T}^{trk} [GeV/c]",
           100, 10, 100,
           100, 0, 150);

  h_nTracks_vs_sumPtTracks = 
    new TH2F("h_nTracks_vs_sumPtTracks",
           "# of tracks vs #Sigma p_{T}^{trk};#Sigma p_{T}^{trk} [GeV/c];Number of Tracks",
           100, 0, 150,
           50, 0, 50);

  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
float rtrk::CalculateDeltaR(float eta1, float phi1,
                            float eta2, float phi2)
{
  float deta = eta1 - eta2;
  float dphi = phi1 - phi2;
  while (dphi >  M_PI) dphi -= 2 * M_PI;
  while (dphi < -M_PI) dphi += 2 * M_PI;
  return std::sqrt(deta * deta + dphi * dphi);
}

//____________________________________________________________________________..
bool rtrk::processVertex(PHCompositeNode *topNode)
{
  GlobalVertexMap *vertexmap = findNode::getClass<GlobalVertexMap>(topNode, "GlobalVertexMap");
  if (!vertexmap || vertexmap->empty()) return false;

  float event_z_vertex = 0.0;
  bool found_mbd_vertex = false;

  for (auto it = vertexmap->begin(); it != vertexmap->end(); ++it) 
  {
    GlobalVertex* vertex = it->second;
    if (vertex && vertex->get_id() == GlobalVertex::MBD) {
      event_z_vertex = vertex->get_z();
      found_mbd_vertex = true;
      break;
    }
  }
  
  if (!found_mbd_vertex) {
    GlobalVertex* vertex = vertexmap->begin()->second;
    if (vertex) event_z_vertex = vertex->get_z();
  }

  h_zvtx_all->Fill(event_z_vertex);
  
  if (fabs(event_z_vertex) > 10.0) return false;

  h_zvtx_cut->Fill(event_z_vertex);
  return true;
}

//____________________________________________________________________________..
int rtrk::processRtrkAnalysis(PHCompositeNode *topNode)
{
  JetContainer* jets = findNode::getClass<JetContainer>(topNode, m_recoJetNameR04);
  SvtxTrackMap *trackmap = findNode::getClass<SvtxTrackMap>(topNode, "SvtxTrackMap");
  RawTowerGeomContainer *geomEM = findNode::getClass<RawTowerGeomContainer>(topNode, "TOWERGEOM_CEMC");

  if (!jets || !trackmap || !geomEM) return Fun4AllReturnCodes::ABORTEVENT;
  
  for (auto jet : *jets)
  {
    if (!jet) continue;

    float jet_pt = jet->get_pt();
    float jet_eta = jet->get_eta();
    float jet_phi = jet->get_phi();
    
    bool eta_cut = (jet_eta >= m_etaRange.first) && (jet_eta <= m_etaRange.second);
    bool pt_cut = (jet_pt >= m_ptRange.first) && (jet_pt <= m_ptRange.second);
    if (!eta_cut || !pt_cut) continue;

    m_total_jets++;
    
    h_jet_eta_phi->Fill(jet_eta, jet_phi);
    h_jet_pt->Fill(jet_pt);
    
    float sum_trk_pt = 0.0;
    int   n_match    = 0;
    
    for (const auto& pair : *trackmap)
    {
      SvtxTrack* track = pair.second;
      if (!track) continue;
      if (track->get_crossing() != 0) continue;
      
      float track_pt = track->get_pt();
      float track_quality = track->get_quality();
      
      if (track_pt < m_trk_pt_min) continue;
      if (track_quality > m_trk_quality_max) continue;
      
      h_track_pt->Fill(track_pt);
      
      SvtxTrackState *emcal_front = track->get_state(geomEM->get_radius());
      SvtxTrackState *emcal_back = track->get_state(geomEM->get_radius() + geomEM->get_thickness());
      if (!emcal_front || !emcal_back) continue;
      
      TVector3 proj_front(emcal_front->get_x(), emcal_front->get_y(), emcal_front->get_z());
      TVector3 proj_back(emcal_back->get_x(), emcal_back->get_y(), emcal_back->get_z());
      TVector3 proj_mean = 0.5 * (proj_front + proj_back);
      float track_proj_eta = proj_mean.Eta();
      float track_proj_phi = proj_mean.Phi();
      
      float dR = CalculateDeltaR(track_proj_eta, track_proj_phi, jet_eta, jet_phi);
      
      h_deltaR_emcal_jet_track->Fill(dR);
      h_deta_emcal_jet_track->Fill(track_proj_eta - jet_eta);
      
      float dphi = track_proj_phi - jet_phi;
      while (dphi >  M_PI) dphi -= 2*M_PI;
      while (dphi < -M_PI) dphi += 2*M_PI;
      h_dphi_emcal_jet_track->Fill(dphi);
      
      h_track_eta_phi_emcal->Fill(track_proj_eta, track_proj_phi);

      if (dR < m_matching_radius)
      {
        sum_trk_pt += track_pt;
        n_match++;
      }
    }
    
    if (n_match > 0)
    {
      h_nTracks_vs_sumPtTracks->Fill(sum_trk_pt, n_match);
    }

    if (n_match > 0 && sum_trk_pt > 0)
    {
      float r_trk = sum_trk_pt / jet_pt;
      h_rtrk_vs_jet_pt_emcal->Fill(jet_pt, r_trk);
      h_sumtrkpt_vs_jetpt->Fill(jet_pt, sum_trk_pt);
      h_nMatchedTracks_emcal->Fill(n_match);
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int rtrk::process_event(PHCompositeNode *topNode)
{

  m_total_events++;

  if (!processVertex(topNode)) {
    return Fun4AllReturnCodes::EVENT_OK;
  }

  int result = processRtrkAnalysis(topNode);
  
  if (m_total_events % 200 == 0) 
  {
    cout << "rtrk::process_event - Done processing " << m_total_events << " events" << endl;
  }

  return result;
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int rtrk::End(PHCompositeNode*)
{
  TFile fout(m_outputFileName.c_str(), "RECREATE");

  h_rtrk_vs_jet_pt_emcal->Write();
  h_nMatchedTracks_emcal->Write();
  h_deltaR_emcal_jet_track->Write();
  h_deta_emcal_jet_track->Write();
  h_dphi_emcal_jet_track->Write();
  h_track_eta_phi_emcal->Write();
  h_jet_eta_phi->Write();
  h_zvtx_all->Write();
  h_zvtx_cut->Write();
  h_jet_pt->Write();
  h_track_pt->Write();
  h_sumtrkpt_vs_jetpt->Write();
  h_nTracks_vs_sumPtTracks->Write();

  fout.Close();
  cout << "rtrk finished\n"
       << "  Events: " << m_total_events << "\n"
       << "  Jets:   " << m_total_jets << endl;

  return Fun4AllReturnCodes::EVENT_OK;
}


