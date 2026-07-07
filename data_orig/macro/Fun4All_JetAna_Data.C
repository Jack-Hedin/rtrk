// _______________________________________________________________________________________________________________________________________
// =======================================================================================================================================
// ---------------------------------------------------------------------------------------------------------------------------------------
// This is the code for 1:10 calo to track ratio using the CALOFITTING. It has extra calibrations 
// _______________________________________________________________________________________________________________________________________
// ---------------------------------------------------------------------------------------------------------------------------------------
// =======================================================================================================================================

#include <G4_ActsGeom.C>
#include <G4_Global.C>
#include <G4_Magnet.C>
#include <GlobalVariables.C>
#include <Trkr_Clustering.C>
#include <Trkr_Reco.C>
#include <Trkr_RecoInit.C>
#include <Trkr_TpcReadoutInit.C>
#include <QA.C>
#include <Calo_Calib.C>  
#include <caloreco/CaloGeomMapping.h>  
#include <ffamodules/CDBInterface.h>

#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllUtils.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllRunNodeInputManager.h>
#include <fun4all/Fun4AllNoSyncDstInputManager.h> //added this

#include <phool/recoConsts.h>
#include <trackreco/PHActsTrackProjection.h>
#include <trackbase_historic/SvtxTrack.h>

#include </sphenix/user/abrahma/datartrk/src/datartrk.h>   // My module
#include<HIJetReco.C>
#include<NoBkgdSubJetReco.C>

#include <iostream>
#include <string>
#include <iomanip>
#include <vector>  

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libphool.so)
R__LOAD_LIBRARY(libcdbobjects.so)
R__LOAD_LIBRARY(libtrack_reco.so)
R__LOAD_LIBRARY(libcalo_reco.so)
R__LOAD_LIBRARY(libcalotrigger.so)
R__LOAD_LIBRARY(libcentrality.so)
R__LOAD_LIBRARY(libmbd.so)
R__LOAD_LIBRARY(libepd.so)
R__LOAD_LIBRARY(libzdcinfo.so)
R__LOAD_LIBRARY(libJetFrag2.so)

// Utility to read filelists
std::string getFileFromList(const std::string& listfile, int segment)
{
  std::ifstream file(listfile);
  std::string line;
  int current_line = 0;
  while (std::getline(file, line))
  {
    if (current_line == segment)
    {
      file.close();
      return line;
    }
    current_line++;
  }
  file.close();
  return "";
}

// Get multiple track files for a given calo segment
std::vector<std::string> getTrackFilesForCaloSegment(const std::string& tracklistfile, int calo_segment)
{
  std::vector<std::string> track_files;
  std::ifstream file(tracklistfile);
  std::string line;
  
  // Each calo segment corresponds to 10 track segments
  int start_track_segment = calo_segment * 10;
  int end_track_segment = start_track_segment + 9;
  
  int current_line = 0;
  while (std::getline(file, line))
  {
    if (current_line >= start_track_segment && current_line <= end_track_segment)
    {
      track_files.push_back(line);
    }
    if (current_line > end_track_segment)
    {
      break;
    }
    current_line++;
  }
  file.close();
  
  return track_files;
}

void Fun4All_JetAna_Data(const int nEvents = -1,
                         const int calo_segment = 0,
                         const std::string &filelistcalo = "calo_filelist_run_53877.txt",
                         const std::string &filelisttracks = "track_filelist_run_53877.txt")
{
  // Read calo file
  std::string inputCaloFile = getFileFromList(filelistcalo, calo_segment);
  
  // Read corresponding track files (10 files per calo segment)
  std::vector<std::string> inputTrackFiles = getTrackFilesForCaloSegment(filelisttracks, calo_segment);

  if (inputCaloFile.empty() || inputTrackFiles.empty())
  {
    std::cout << "ERROR: Missing input file(s) for calo segment " << calo_segment << std::endl;
    std::cout << "  Calo file: " << inputCaloFile << std::endl;
    std::cout << "  Number of track files: " << inputTrackFiles.size() << std::endl;
    return;
  }

  std::pair<int, int> runseg = Fun4AllUtils::GetRunSegment(inputCaloFile);
  int runnumber = runseg.first;
  int segnum = runseg.second;

  std::cout << "=== Job Configuration ===" << std::endl;
  std::cout << "Calo Segment: " << calo_segment << std::endl;
  std::cout << "Using input CALO file: " << inputCaloFile << std::endl;
  std::cout << "Using " << inputTrackFiles.size() << " input TRACK files:" << std::endl;
  for (size_t i = 0; i < inputTrackFiles.size(); i++)
  {
    std::cout << "  Track file " << i << ": " << inputTrackFiles[i] << std::endl;
  }

  // Output filename
  std::string outfilename = Form("/sphenix/user/abrahma/datartrk/macro/output_data/R_Tracks_Data_run%d_calo_seg_%05d.root", runnumber, calo_segment);
  std::cout << "Output file: " << outfilename << std::endl;

  // === Setup Fun4All ===
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  Enable::MVTX_APPLYMISALIGNMENT = true;
  ACTSGEOM::mvtx_applymisalignment = Enable::MVTX_APPLYMISALIGNMENT;
  TRACKING::pp_mode = true;

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", runnumber);
  rc->set_IntFlag("RUNSEGMENT", segnum);

  //  Register inputs BEFORE geometry 
  
  // Register one track file
  //auto trackin = new Fun4AllDstInputManager("TrackInManager_0");
  //trackin->fileopen(inputTrackFiles[0]);
  //se->registerInputManager(trackin);

  // Register ALL track files
  /*for (size_t i = 0; i < inputTrackFiles.size(); i++)
  {
    auto trackin = new Fun4AllDstInputManager(Form("TrackInManager_%zu", i));
    trackin->AddFile(inputTrackFiles[i]);
    se->registerInputManager(trackin);
    std::cout << "Registered track file " << i << ": " << inputTrackFiles[i] << std::endl;
  }*/

  /*for (size_t i = 0; i < inputTrackFiles.size(); i++)
  {
    auto trackin = new Fun4AllDstInputManager(Form("TrackInManager_%zu", i));
    trackin->fileopen(inputTrackFiles[i]);  // Use fileopen() instead of AddFile()
    se->registerInputManager(trackin);
    std::cout << "Registered track file " << i << ": " << inputTrackFiles[i] << std::endl;
  }*/

  // Register calo file
  /*auto caloin = new Fun4AllDstInputManager("CaloInManager");
  caloin->fileopen(inputCaloFile);  
  se->registerInputManager(caloin);
  std::cout << "Registered calo file: " << inputCaloFile << std::endl;*/

  // Create ONE input manager for all track files
  auto trackin = new Fun4AllDstInputManager("TrackInManager");
  for (size_t i = 0; i < inputTrackFiles.size(); i++)
  {
    trackin->AddFile(inputTrackFiles[i]);
    std::cout << "Added track file " << i << " to manager: " << inputTrackFiles[i] << std::endl;
  }
  se->registerInputManager(trackin);

  // Register calo file with separate manager
  auto caloin = new Fun4AllDstInputManager("CaloInManager");
  caloin->AddFile(inputCaloFile);
  se->registerInputManager(caloin);
  std::cout << "Registered calo file: " << inputCaloFile << std::endl;


  std::cout << "=== Track and Calo Files register setup complete ===" << std::endl;

  // === NOW load geometry ===
  Enable::CDB = true;
  rc->set_StringFlag("CDB_GLOBALTAG", "2024p023");
  rc->set_uint64Flag("TIMESTAMP", runnumber);
  std::string geofile = CDBInterface::instance()->getUrl("Tracking_Geometry");
  std::cout << "Geometry file from CDB: " << geofile << std::endl;
  Fun4AllRunNodeInputManager *ingeo = new Fun4AllRunNodeInputManager("GeoIn");
  ingeo->AddFile(geofile);
  se->registerInputManager(ingeo);
  
  TpcReadoutInit(runnumber);

  // === Initialize tracking ===
  TrackingInit();
  Global_Reco();

  // === Calo Geometry Mapping - MUST come before calo reconstruction ===
  CaloGeomMapping *cgm = new CaloGeomMapping("CEMC_DETAILED");
  cgm->set_detector_name("CEMC");
  cgm->set_UseDetailedGeometry(true);
  se->registerSubsystem(cgm);

  // === Calo Calibration ===
  Process_Calo_Calib();
  std::cout << "Calo calib run" << std::endl;

  // === Track projection - NOW it should find the geometry ===
  auto projection = new PHActsTrackProjection("CaloProjection");
  se->registerSubsystem(projection);

  std::cout << "=== Geometry and Calibrations registered and complete ===" << std::endl;
  
  // === Jet Reconstruction ===
  /*Enable::HIJETS = true;
  Enable::HIJETS_MC = false;
  Enable::HIJETS_TRUTH = false;
  HIJetReco();*/
  Enable::NSJETS = true;
  Enable::NSJETS_TOWER = true;
  Enable::NSJETS_MC = false;
  Enable::NSJETS_TRUTH = false;
  NoBkgdSubJetReco();
  
  std::cout << "=== Jet Reconstruction complete ===" << std::endl;

  // === Your analysis module ===
  datartrk *ana = new datartrk("AntiKt_Tower_r04", outfilename.c_str());
  se->registerSubsystem(ana);
  std::cout << "Registered my module" << std::endl;

  // === Run ===
  se->run(nEvents);
  se->End();
  se->PrintTimer();

  std::cout << "Finished Data Rtrk analysis for calo segment " << calo_segment << std::endl;
  gSystem->Exit(0);
}
