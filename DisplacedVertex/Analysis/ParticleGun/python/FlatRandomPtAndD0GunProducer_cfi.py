import FWCore.ParameterSet.Config as cms

generator = cms.EDProducer("FlatRandomPtAndD0GunProducer",
                           PGunParameters = cms.PSet(
    MaxPt = cms.double(100.01),
    MinPt = cms.double(99.99),
    PartID = cms.vint32(-13),
    MaxEta = cms.double(2.5),
    MaxPhi = cms.double(1.57),
    MinEta = cms.double(-2.5),
    # MinPhi = cms.double(-3.14159265359), ## in radians
    MinPhi = cms.double(1.58), ## in radians
    D0  = cms.double(0.),
    ),
                           Verbosity = cms.untracked.int32(0), ## set to 1 (or greater)  for printouts
                           
                           psethack = cms.string('single mu pt 100'),
                           AddAntiParticle = cms.bool(True),
                           firstRun = cms.untracked.uint32(1)
                           )
