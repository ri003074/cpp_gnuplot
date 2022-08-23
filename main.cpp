#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

int getNearestIndex(vector<double> data, double d)
{
    double diff = 999;
    int index = 999;

    for (int i = 0; i < data.size(); i++)
    {
        double _diff = abs(d - data[i]);

        if (diff > _diff)
        {
            index = i;
            diff = _diff;
        }
    }
    return index;
}

int StringToInt(string str)
{
    stringstream ss;
    int num;
    ss << str;
    ss >> num;
    return num;
}

double StringToDouble(string str)
{
    stringstream ss;
    double num;
    ss << str;
    ss >> num;
    return num;
}

vector<string> split(string s, string delimiter)
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != string::npos)
    {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

class Gnuplot
{
private:
    vector<vector<string> > shmErrorCount;
    vector<double> shmYAxis;
    vector<double> shmXAxis;

    vector<double> failCountList;

    string paletteDefine;
    string cbtics;
    string xtics;
    string ytics;
    int xIndexCount;
    int yIndexCount;

    string inputFileName;
    string intermediateFileName;

    int maxFailCount;

    const static int xAxisDigits = 5;
    const static int yAxisDigits = 5;

    void generateIntermediateFile();
    void calcFailCountList();
    void calcPaletteDefined();
    void calcCbtics();
    void calcXtics();
    void calcYtics();

public:
    void readShmFile(string fileName);
    void plot();
};

void Gnuplot::readShmFile(string fileName)
{
    inputFileName = fileName;

    ifstream ifs;
    ifs.open(inputFileName, ios::in);

    string line;
    bool isFirstLine = true;
    while (getline(ifs, line))
    {
        if (isFirstLine)
        {
            isFirstLine = false;
            vector<string> tmp;
            tmp = split(line, ",");
            for (int i = 0; i < tmp.size(); i++)
            {
                shmXAxis.push_back(StringToDouble(tmp[i]));
            }
        }
        else
        {
            vector<string> tmp;
            tmp = split(line, ":");
            shmYAxis.push_back(StringToDouble(tmp[0]));
            vector<string> shmLine;
            shmLine = split(tmp[1], " ");
            shmErrorCount.push_back(shmLine);
        }
    }

    vector<double> tmp;
    tmp = shmYAxis;
    for (int i = 0; i < shmYAxis.size(); i++)
    {
        shmYAxis[i] = tmp[shmYAxis.size() - 1 - i];
    }
}

void Gnuplot::generateIntermediateFile()
{
    maxFailCount = -1;

    ofstream output;
    intermediateFileName = inputFileName + "_intermediate";
    output.open(intermediateFileName, ios::out);

    for (int y = 0; y < shmErrorCount.size(); y++)
    {
        for (int x = 0; x < shmErrorCount[y].size(); x++)
        {
            if (y == shmErrorCount.size() - 1)
            {
                int failCount = StringToInt(shmErrorCount[y][x]) - StringToInt(shmErrorCount[y - 1][x]);
                output << failCount;
                output << " ";

                if (failCount > maxFailCount)
                {
                    maxFailCount = failCount;
                }
            }
            else
            {
                int failCount = abs(StringToInt(shmErrorCount[y][x]) - StringToInt(shmErrorCount[y + 1][x]));
                output << failCount;
                output << " ";

                if (failCount > maxFailCount)
                {
                    maxFailCount = failCount;
                }
            }
        }
        output << endl;
    }
}

void Gnuplot::calcFailCountList()
{
    failCountList.push_back(0);
    failCountList.push_back(0.9);

    double resolution = double(maxFailCount) / 10.0;

    // TODO double check calculation
    for (double i = resolution; i <= maxFailCount + resolution; i += resolution)
    {
        failCountList.push_back(int(i));
    }
}

void Gnuplot::calcPaletteDefined()
{
    paletteDefine = "";
    for (int i = 0; i <= 10; i++)
    { //TODO 10?
        ostringstream oss;
        oss << hex << int(255 - i * 25.5);
        string result = oss.str();

        if (result.length() == 1)
        {
            result = "0" + result;
        }
        paletteDefine += to_string(failCountList[i]) + " '#" + result + result + "ff'";
        if (i != 10)
        {
            paletteDefine += ",";
        }
    }
}

void Gnuplot::calcCbtics()
{
    cbtics = "";

    for (int i = 0; i < failCountList.size(); i++)
    {
        if (i == failCountList.size() - 1)
        {
            cbtics += "\"" + to_string(failCountList[i]) + "\"" + to_string(failCountList[i]);
        }
        else if (i == 0)
        {
            cbtics += "\"\"" + to_string(failCountList[i]) + ",";
        }
        else
        {
            cbtics += "\"" + to_string(failCountList[i]) + "\"" + to_string(failCountList[i]) + ",";
        }
    }
}

void Gnuplot::calcXtics()
{
    xtics = "";
    xIndexCount = 0;

    double minIndexValue = shmXAxis[0];
    double maxIndexValue = shmXAxis[shmXAxis.size() - 1];
    double resolution = (maxIndexValue - minIndexValue) / 8.0;

    for (double d = minIndexValue; d < maxIndexValue + resolution; d += resolution)
    {
        if (d == minIndexValue)
        {
            if (d < 0)
            {
                xtics += "\"" + to_string(d).substr(0, xAxisDigits + 1) + "\"" + to_string(getNearestIndex(shmXAxis, d));
            }
            else
            {
                xtics += "\"" + to_string(d).substr(0, xAxisDigits) + "\"" + to_string(getNearestIndex(shmXAxis, d));
            }
        }
        else
        {
            if (d < 0)
            {
                xtics += ", \"" + to_string(d).substr(0, xAxisDigits + 1) + "\"" + to_string(getNearestIndex(shmXAxis, d));
            }
            else
            {
                xtics += ", \"" + to_string(d).substr(0, xAxisDigits) + "\"" + to_string(getNearestIndex(shmXAxis, d));
            }
            xIndexCount = getNearestIndex(shmXAxis, d);
        }
    }
}

void Gnuplot::calcYtics()
{
    ytics = "";
    yIndexCount = 0;

    for (int i = 0; i < shmYAxis.size(); i++)
    {
        cout << shmYAxis[i] << endl;
    }

    //TODO refactoring!!!
    if (shmYAxis[0] > shmYAxis[shmYAxis.size() - 1])
    {
        double minIndexValue = shmYAxis[shmYAxis.size() - 1];
        double maxIndexValue = shmYAxis[0];
        double resolution = (maxIndexValue - minIndexValue) / 8.0;
        for (double d = maxIndexValue; d > minIndexValue - resolution; d -= resolution)
        {
            if (d == maxIndexValue)
            {
                ytics += "\"" + to_string(d).substr(0, yAxisDigits) + "\"" + to_string(getNearestIndex(shmYAxis, d));
            }
            else
            {
                ytics += ", \"" + to_string(d).substr(0, yAxisDigits) + "\"" + to_string(getNearestIndex(shmYAxis, d));
                yIndexCount = getNearestIndex(shmYAxis, d);
            }
        }
    }
    else
    {
        double minIndexValue = shmYAxis[0];
        double maxIndexValue = shmYAxis[shmYAxis.size() - 1];
        double resolution = (maxIndexValue - minIndexValue) / 8.0;
        for (double d = minIndexValue; d < maxIndexValue + resolution; d += resolution)
        {
            if (d == minIndexValue)
            {
                ytics += "\"" + to_string(d).substr(0, yAxisDigits) + "\"" + to_string(getNearestIndex(shmYAxis, d));
            }
            else
            {
                ytics += ", \"" + to_string(d).substr(0, yAxisDigits) + "\"" + to_string(getNearestIndex(shmYAxis, d));
                yIndexCount = getNearestIndex(shmYAxis, d);
            }
        }
    }
}

void Gnuplot::plot()
{
    generateIntermediateFile();
    calcFailCountList();
    calcPaletteDefined();
    calcCbtics();
    calcXtics();
    calcYtics();

    FILE *gp;
    gp = popen("gnuplot -persist", "w");

    vector<string> gnuplotCommand;
    gnuplotCommand.push_back("set pm3d map\n");
    gnuplotCommand.push_back("set xrange[0:" + to_string(xIndexCount) + "]\n");
    gnuplotCommand.push_back("set xtics (" + xtics + ")\n");
    gnuplotCommand.push_back("set yrange[0:" + to_string(yIndexCount) + "]\n");
    gnuplotCommand.push_back("set ytics (" + ytics + ")\n");
    gnuplotCommand.push_back("set palette defined (" + paletteDefine + ")\n");
    gnuplotCommand.push_back("set cbrange[0:" + to_string(maxFailCount) + "]\n");
    gnuplotCommand.push_back("set cbtics (" + cbtics + ")\n");
    gnuplotCommand.push_back("splot \"" + intermediateFileName + "\" matrix with pm3d\n");

    for (int i = 0; i < gnuplotCommand.size(); i++)
    {
        fprintf(gp, "%s\n", gnuplotCommand[i].c_str());
        cout << gnuplotCommand[i];
    }

    pclose(gp);
}

int main()
{
    Gnuplot gp;
    gp.readShmFile("data3.txt");
    gp.plot();
}
