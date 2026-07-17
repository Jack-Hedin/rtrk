#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <iostream>

void TheGreatPlotini(const char* infile = "/sphenix/user/abrahma/datartrk/macro/output_data/test1.root")
{
    // ________________________________________________________________________________________________
    // Open file
    // ________________________________________________________________________________________________

    TFile* f = TFile::Open(infile, "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    // ==================== MATCHED TRACKS ====================
    TH1F* h_nMatched_emcal = (TH1F*) f->Get("h_nMatchedTracks_emcal");
    if (h_nMatched_emcal) {
        TCanvas* c_n = new TCanvas("c_n","Matched Tracks",800,800);
        h_nMatched_emcal->SetTitle("Matched Tracks per Jet;N_{matched};Counts");
        h_nMatched_emcal->Draw();
        c_n->SaveAs("h_nMatchedTracks_emcal.png");
    }

    // ==================== JET PT ====================
    TH1F* h_jet_pt = (TH1F*) f->Get("h_jet_pt");
    if (h_jet_pt) {
        TCanvas* c_jet_pt = new TCanvas("c_jet_pt", "Jet p_{T}", 800, 800);
        h_jet_pt->SetTitle("Jet p_{T} Distribution;Jet p_{T} [GeV/c];Counts");
        h_jet_pt->Draw();
        c_jet_pt->SaveAs("Jet_pt_distribution.png");
    }

    // ==================== TRACK PT ====================
    TH1F* h_track_pt = (TH1F*) f->Get("h_track_pt");
    if (h_track_pt) {
        TCanvas* c_track_pt = new TCanvas("c_track_pt", "Track p_{T}", 800, 800);
        h_track_pt->SetTitle("Track p_{T} Distribution;Track p_{T} [GeV/c];Counts");
        h_track_pt->Draw();
        c_track_pt->SaveAs("track_pt_distribution.png");
    }

    // ==================== R_TRK 2D ====================
    TH2F* h_rtrk_emcal = (TH2F*) f->Get("h_rtrk_vs_jet_pt_emcal");
    if (h_rtrk_emcal) 
    {
        // 2D plot
        TCanvas* c_raw_emcal = new TCanvas("c_raw_emcal","r_{trk} vs Jet p_{T}",800,800);
        //h_rtrk_emcal->SetTitle("r_{trk} vs Jet p_{T};Jet p_{T} [GeV/c];r_{trk}");
        h_rtrk_emcal->Draw("COLZ");
        c_raw_emcal->SaveAs("h_rtrk_vs_jet_pt_emcal.png");
        
        // Profile plot
        TProfile* prof_rtrk_emcal = h_rtrk_emcal->ProfileX("prof_rtrk_emcal");
        TCanvas* c_prof = new TCanvas("c_prof","<r_{trk}> vs Jet p_{T}",800,800);
        prof_rtrk_emcal->SetTitle("<r_{trk}> vs Jet p_{T};Jet p_{T} [GeV/c];<r_{trk}>");
        prof_rtrk_emcal->SetMarkerStyle(20);
        prof_rtrk_emcal->SetMarkerSize(1);
        prof_rtrk_emcal->Draw("E1");
        c_prof->SaveAs("p_rtrk_emcal.png");
    }

    // ==================== DELTA R/ETA/PHI ====================
    TH1F* h_deltaR = (TH1F*) f->Get("h_deltaR_emcal_jet_track");
    TH1F* h_deta = (TH1F*) f->Get("h_deta_emcal_jet_track");
    TH1F* h_dphi = (TH1F*) f->Get("h_dphi_emcal_jet_track");
    
    if (h_deltaR) {
        TCanvas* c_dR = new TCanvas("c_dR","#DeltaR(jet,track)",800,800);
        h_deltaR->Draw();
        c_dR->SaveAs("h_deltaR_emcal_jet_track.png");
    }
    
    if (h_deta) {
        TCanvas* c_deta = new TCanvas("c_deta","#Delta#eta(jet,track)",800,800);
        h_deta->Draw();
        c_deta->SaveAs("h_deta_emcal_jet_track.png");
    }

    if (h_dphi) {
        TCanvas* c_dphi = new TCanvas("c_dphi","#Delta#phi(jet,track)",800,800);
        h_dphi->Draw();
        c_dphi->SaveAs("h_dphi_emcal_jet_track.png");
    }

    // ==================== SUM TRK PT VS JET PT ====================
    TH2F* h_sumtrkpt_vs_jetpt = (TH2F*) f->Get("h_sumtrkpt_vs_jetpt");
    if (h_sumtrkpt_vs_jetpt) 
    {
        // 2D plot
        TCanvas* c_sumtrk = new TCanvas("c_sumtrk","#Sigma p_{T}^{trk} vs Jet p_{T}",800,800);
        h_sumtrkpt_vs_jetpt->Draw("COLZ");
        c_sumtrk->SaveAs("h_sumtrkpt_vs_jetpt.png");
        
        // Profile plot
        TProfile* prof_sumtrk = h_sumtrkpt_vs_jetpt->ProfileX("prof_sumtrk");
        TCanvas* c_prof_sum = new TCanvas("c_prof_sum","<#Sigma p_{T}^{trk}> vs Jet p_{T}",800,800);
        prof_sumtrk->SetMarkerStyle(20);
        prof_sumtrk->SetMarkerSize(1);
        prof_sumtrk->Draw("E1");
        c_prof_sum->SaveAs("p_sumtrkpt_vs_jetpt.png");
    }

  
    // ==================== JET ETA-PHI ====================
    TH2F* h_jet_eta_phi = (TH2F*) f->Get("h_jet_eta_phi");
    if (h_jet_eta_phi) {
        TCanvas* c_j = new TCanvas("c_j","Jet #eta-#phi",800,800);
        h_jet_eta_phi->SetTitle("Jet #eta-#phi;#eta;#phi");
        h_jet_eta_phi->Draw("COLZ");
        c_j->SaveAs("h_jet_eta_phi.png");
    }

    // ==================== TRACK ETA-PHI ====================
    TH2F* h_track_eta_phi_emcal = (TH2F*) f->Get("h_track_eta_phi_emcal");
    if (h_track_eta_phi_emcal) {
        TCanvas* c_te = new TCanvas("c_te","Track #eta-#phi",800,800);
        h_track_eta_phi_emcal->SetTitle("Track #eta-#phi;#eta;#phi");
        h_track_eta_phi_emcal->Draw("COLZ");
        c_te->SaveAs("h_track_eta_phi_emcal.png");
    }
  
    // ==================== Z-VERTEX ====================
    TH1F* h_zvtx_all = (TH1F*) f->Get("h_zvtx_all");
    TH1F* h_zvtx_cut = (TH1F*) f->Get("h_zvtx_cut");
    
    if (h_zvtx_all && h_zvtx_cut) {
        TCanvas* c_zvtx = new TCanvas("c_zvtx","Z-vertex",800,800);
        h_zvtx_all->SetLineColor(kBlack);
        h_zvtx_all->SetLineWidth(2);
        h_zvtx_all->SetTitle("Z-vertex Distributions;z_{vtx} [cm];Counts");
        h_zvtx_all->Draw("HIST");
        h_zvtx_cut->SetLineColor(kRed);
        h_zvtx_cut->SetLineWidth(2);
        h_zvtx_cut->Draw("HIST SAME");
        
        TLegend* leg_z = new TLegend(0.6,0.75,0.9,0.88);
        leg_z->AddEntry(h_zvtx_all,"All events","l");
        leg_z->AddEntry(h_zvtx_cut,"|z_{vtx}| < 10 cm","l");
        leg_z->Draw();
        c_zvtx->SaveAs("h_zvtx_comparison.png");
    }

    // ==================== TRIGGER PLOTS (if data) ====================
    TH1F* h_trigger_22_23 = (TH1F*) f->Get("h_trigger_22_23");
    if (h_trigger_22_23) {
        std::cout << "Data file - plotting trigger histograms" << std::endl;
        
        // Trigger 22/23
        TCanvas* c_trig = new TCanvas("c_trig","Triggers 22 & 23",800,800);
        h_trigger_22_23->SetTitle("Triggers 22 & 23;Trigger;Counts");
        h_trigger_22_23->SetFillColor(kBlue);
        h_trigger_22_23->Draw("HIST");
        c_trig->SaveAs("h_trigger_22_23.png");
        // Trigger decision
        TH1F* h_trigger_decision = (TH1F*) f->Get("h_trigger_decision");
        if (h_trigger_decision) {
            TCanvas* c_trig_dec = new TCanvas("c_trig_dec","Trigger Decision",800,800);
            h_trigger_decision->SetTitle("Trigger Decision;0=NoTrig,1=Trig22/23,2=OtherTrig;Counts");
            h_trigger_decision->SetFillColor(kGreen);
            h_trigger_decision->Draw("HIST");
            c_trig_dec->SaveAs("h_trigger_decision.png");
        } 
        // Trigger efficiency
        TH1F* h_events_passed = (TH1F*) f->Get("h_events_passed_triggers");
        if (h_events_passed) {
            TCanvas* c_trig_eff = new TCanvas("c_trig_eff","Trigger Efficiency",800,800);
            h_events_passed->SetTitle("Trigger Efficiency;0=All,1=Passed;Counts");
            h_events_passed->SetFillColor(kOrange);
            h_events_passed->Draw("HIST");
            c_trig_eff->SaveAs("h_events_passed_triggers.png");
        }
    }

    // ================================================================================================
    // END
    // ------------------------------------------------------------------------------------------------
    
    f->Close();
    std::cout << "All plots saved!" << std::endl;
}
    
// _____________________________________________________________________________________________________



    
