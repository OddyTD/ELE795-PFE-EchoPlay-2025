#include <iostream>
#include <fstream>
#include <iomanip>
#include <limits>
#include <vector>
#include <algorithm>

/****************************************************************************
*                                                                           *
*   CardsDisposition.cpp                                                    * 
*   C++ program to calculate optimal card layout for a display area         *
*   This program evaluates different configurations of card layouts         *
*   based on specified parameters and outputs the results to a CSV file.    *
*   The configurations include varying the number of rows, columns, and     *
*   It finds the configuration with the smallest positive total loss        *
*   of space in the display area, considering margins and card dimensions.  *
*                                                                           *
****************************************************************************/

// ---- Struct to hold one result row for CSV output ----
struct LayoutResult {
    int rows, columns;             // Number of rows and columns
    int marginBetween, marginEdge; // Margins between cards and at screen edges
    double cardHeight, cardWidth;  // Dimensions of one card
    double usedHeight, lossHeight; // Used and leftover vertical space
    double usedWidth, lossWidth;   // Used and leftover horizontal space
    double lossTotal;              // Combined unused space in both dimensions
};

/********************************************************
*                                                       *
* Comparison function for sorting layout results        *
* by increasing lossTotal (used for finding best fits). *
*                                                       *
********************************************************/
bool compareByLoss(const LayoutResult& a, const LayoutResult& b) {
    return a.lossTotal < b.lossTotal;
}

int main() {
    // ------------------------------------------------------------
    // CONFIGURATION SECTION
    // ------------------------------------------------------------

    const int screenHeight = 320;     // Fixed height of the display area (e.g., touchscreen)
    const int screenWidth = 480;      // Fixed width of the display area
    const double cardRatio = 0.72;    // Aspect ratio: width = height * ratio

    // Data containers
    std::vector<LayoutResult> allResults;   // All combinations
    std::vector<LayoutResult> validResults; // Only valid ones with lossTotal >= 0

    // ------------------------------------------------------------
    // EXHAUSTIVE SEARCH: Try all combinations
    // ------------------------------------------------------------

    for (int rows = 2; rows <= 4; ++rows) {
        for (int marginBetween = 3; marginBetween <= 10; ++marginBetween) {
            for (int marginEdge = 3; marginEdge <= 10; ++marginEdge) {

                // Calculate card height and width from screen height and layout
                double cardHeight = (screenHeight - 2 * marginEdge - (rows - 1) * marginBetween)
                                    / static_cast<double>(rows);
                double cardWidth = cardHeight * cardRatio;

                // Total vertical usage and loss
                double usedHeight = rows * cardHeight + (rows - 1) * marginBetween + 2 * marginEdge;
                double lossHeight = screenHeight - usedHeight;

                // Try every number of columns for this row configuration
                for (int columns = 3; columns <= 10; ++columns) {
                    // Total horizontal usage and loss
                    double usedWidth = columns * cardWidth + (columns - 1) * marginBetween + 2 * marginEdge;
                    double lossWidth = screenWidth - usedWidth;

                    // Combined layout inefficiency
                    double lossTotal = lossHeight + lossWidth;

                    // Store result
                    LayoutResult r{rows, columns, marginBetween, marginEdge,
                                   cardHeight, cardWidth,
                                   usedHeight, lossHeight,
                                   usedWidth, lossWidth,
                                   lossTotal};

                    allResults.push_back(r); // Store all results (for full table)

                    if (lossTotal >= 0) {
                        validResults.push_back(r); // Only store valid layouts for top results
                    }
                }
            }
        }
    }

    // ------------------------------------------------------------
    // PROCESS RESULTS: Find top 10 most space-efficient layouts
    // ------------------------------------------------------------

    std::sort(validResults.begin(), validResults.end(), compareByLoss); // Sort by efficiency

    std::vector<LayoutResult> top10;
    for (size_t i = 0; i < std::min<size_t>(10, validResults.size()); ++i) {
        top10.push_back(validResults[i]);
    }

    // ------------------------------------------------------------
    // CSV OUTPUT: Save results to file
    // ------------------------------------------------------------

    std::ofstream file("results.csv");

    // ---- Write header for top 10 summary ----
    file << "TopResults (sorted by smallest lossTotal):\n";
    file << "Rank,LossTotal,Rows,Columns,MarginBetween,MarginEdge,CardHeight,CardWidth\n";

    for (size_t i = 0; i < top10.size(); ++i) {
        const auto& r = top10[i];
        file << (i + 1) << "," 
             << std::fixed << std::setprecision(2)
             << r.lossTotal << "," << r.rows << "," << r.columns << ","
             << r.marginBetween << "," << r.marginEdge << ","
             << r.cardHeight << "," << r.cardWidth << "\n";
    }

    file << "\n"; // spacing between sections

    // ---- Write full table header ----
    file << "Rows,Columns,MarginBetween,MarginEdge,"
         << "CardHeight,CardWidth,"
         << "UsedHeight,LossHeight,UsedWidth,LossWidth,LossTotal\n";

    // ---- Write all configurations ----
    for (const auto& r : allResults) {
        file << r.rows << "," << r.columns << "," << r.marginBetween << "," << r.marginEdge << ","
             << std::fixed << std::setprecision(2)
             << r.cardHeight << "," << r.cardWidth << ","
             << r.usedHeight << "," << r.lossHeight << ","
             << r.usedWidth << "," << r.lossWidth << ","
             << r.lossTotal << "\n";
    }

    file.close();
    std::cout << "CSV export completed: results.csv" << std::endl;
    return 0;
}
