/* Copyright 2022 Chan Beom Park <cbpark@gmail.com>
 *
 * The decay topology:
 *   A1 + A2 --> a1 B1 + a2 B2 --> a1 b1 C1 + a2 b2 C2,
 * where
 *   C1 and C2 are invisible particles.
 *
 * We assume that the decay topology is symmetric.
 */

#include <TGenPhaseSpace.h>
#include <TLorentzVector.h>
#include <TRandom3.h>
#include <YAM2/yam2.h>
#include <cstdlib>
#include <fstream>
#include <iostream>

const double MA = 1000.0;
const double MB = 800.0;
const double MC = 700.0;

// all the visible particles are massless.
const double MV = 0.0;

int main(int argc, char *argv[]) {
    if (!(argc == 3)) {
        std::cout << "usage: ./bin/m2ccb <output.dat> <nev>\n"
                  << "  <output.dat>: output file name.\n"
                  << "  <nev>: the number of events.\n";
        return 1;
    }

    // We consider a pair production of A1 and A2 at threshold.
    TLorentzVector s = {0.0, 0.0, 0.0, 2.0 * MA};

    TGenPhaseSpace event;
    const double AA[2] = {MA, MA};
    event.SetDecay(s, 2, AA);

    const double BB[2] = {MB, MV};
    const double CC[2] = {MC, MV};

    // set the seed for random number generation.
    gRandom = new TRandom3(42);

    std::ofstream output{argv[1]};
    const auto nev = std::atoi(argv[2]);
    for (auto iev = 0; iev != nev; ++iev) {
        event.Generate();
        auto A1 = event.GetDecay(0);
        auto A2 = event.GetDecay(1);

        // A1 --> B1 + a1, A2 --> B2 + a2.
        TGenPhaseSpace A1_decay, A2_decay;
        A1_decay.SetDecay(*A1, 2, BB);
        A2_decay.SetDecay(*A2, 2, BB);
        A1_decay.Generate();
        A2_decay.Generate();

        auto B1 = A1_decay.GetDecay(0);
        auto a1 = A1_decay.GetDecay(1);
        auto B2 = A2_decay.GetDecay(0);
        auto a2 = A2_decay.GetDecay(1);

        // B1 --> C1 + b1, B2 --> C2 + b2.
        TGenPhaseSpace B1_decay, B2_decay;
        B1_decay.SetDecay(*B1, 2, CC);
        B2_decay.SetDecay(*B2, 2, CC);
        B1_decay.Generate();
        B2_decay.Generate();

        auto C1 = B1_decay.GetDecay(0);
        auto b1 = B1_decay.GetDecay(1);
        auto C2 = B2_decay.GetDecay(0);
        auto b2 = B2_decay.GetDecay(1);

        // The visible particles.
        const yam2::FourMomentum p_a1{a1->E(), a1->Px(), a1->Py(), a1->Pz()};
        const yam2::FourMomentum p_a2{a2->E(), a2->Px(), a2->Py(), a2->Pz()};
        const yam2::FourMomentum p_b1{b1->E(), b1->Px(), b1->Py(), b1->Pz()};
        const yam2::FourMomentum p_b2{b2->E(), b2->Px(), b2->Py(), b2->Pz()};

        // MET.
        const yam2::TransverseMomentum ptmiss{C1->Px() + C2->Px(),
                                              C1->Py() + C2->Py()};
        // std::cout << "ptmiss: " << ptmiss << '\n';

        const auto input =
            yam2::mkInput({p_a1, p_a2}, {p_b1, p_b2}, ptmiss, yam2::Mass{MC});
        if (!input) {
            std::cerr << "Invalid input (" << iev + 1 << ")\n";
            continue;
        }

        const auto m2ccb = yam2::m2CCB(input);
        if (!m2ccb) {
            std::cerr << "Failed to find minimum (" << iev + 1 << ")\n";
            continue;
        } else {
            output << m2ccb.value().m2() << '\n';
        }
    }

    // close the output file.
    output.close();

    // finished.
    std::cout << "-- the number of events processed: " << nev << '\n';
    std::cout << "-- the results have been stored in " << argv[1] << '\n';
}
