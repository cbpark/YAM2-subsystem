#include <TCanvas.h>
#include <TH1.h>
#include <TStyle.h>
#include <fstream>
#include <sstream>
#include <string>

void m2CCB(const char *data) {
    const double xlow = 600.0;
    const double xup = 900.0;
    const int nbins = (xup - xlow) / 2.0;
    TH1D *hist = new TH1D("m2ccb", "", nbins, xlow, xup);
    hist->SetXTitle("M_{2CC}(b)");
    hist->SetYTitle("events");
    hist->GetXaxis()->CenterTitle();
    hist->GetYaxis()->CenterTitle();
    hist->SetTitle("");

    std::ifstream fin{data};
    std::string line;
    while (std::getline(fin, line)) {
        if (line.front() == '#') { continue; }

        std::istringstream iss(line);
        double m2;

        if (!(iss >> m2)) { break; }

        hist->Fill(m2);
    }
    fin.close();

    TCanvas *canvas = new TCanvas("c", "", 0, 0, 600, 600);
    canvas->SetTicks();
    gStyle->SetOptStat();

    hist->Draw();
    canvas->SaveAs("m2ccb.pdf");
}
