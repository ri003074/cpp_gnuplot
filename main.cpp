#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

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
    vector<string> xAxis;
    vector<string> yAxis;
    vector<int> failCountList;
    vector<vector<string> > shmData;
    string paletteDefine;
    string cbtics;
    string xtics;

    string inputFileName;
    string intermediateFileName;

    int maxFailCount;

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
            xAxis = split(line, ",");
        }
        else
        {
            vector<string> tmp;
            tmp = split(line, ":");
            yAxis.push_back(tmp[0]);
            vector<string> shmLine;
            shmLine = split(tmp[1], " ");
            shmData.push_back(shmLine);
        }
    }
}

void Gnuplot::generateIntermediateFile()
{
    maxFailCount = -1;

    ofstream output;
    intermediateFileName = inputFileName + "_intermediate";
    output.open(intermediateFileName, ios::out);

    for (int y = 0; y < shmData.size(); y++)
    {
        for (int x = 0; x < shmData[y].size(); x++)
        {
            if (y == shmData.size() - 1)
            {
                int failCount = StringToInt(shmData[y][x]) - StringToInt(shmData[y - 1][x]);
                output << failCount;
                output << " ";

                if (failCount > maxFailCount)
                {
                    maxFailCount = failCount;
                }
            }
            else
            {
                int failCount = StringToInt(shmData[y][x]) - StringToInt(shmData[y + 1][x]);
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
    failCountList.push_back(1);

    double resolution = double(maxFailCount) / 10.0;

    // TODO double check calculation
    for (double i = resolution; i <= maxFailCount + resolution; i += resolution)
    {
        failCountList.push_back(int(i));
        cout << i << endl;
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

    double minIndexValue = StringToDouble(xAxis[0]);
    double maxIndexValue = StringToDouble(xAxis[xAxis.size() - 1]);

    double resolution = (maxIndexValue - minIndexValue) / 8.0;

    for (double d = minIndexValue; d < maxIndexValue + resolution; d += resolution)
    {
        cout << d << endl;
    }
}

void Gnuplot::plot()
{
    generateIntermediateFile();
    calcFailCountList();
    calcPaletteDefined();
    calcCbtics();
    calcXtics();

    FILE *gp;
    gp = popen("gnuplot -persist", "w");
    fprintf(gp, "set pm3d map\n");
    fprintf(gp, "set xrange[0:100]\n");
    fprintf(gp, "set xtics (\"-1\" 0,\"0\" 50,\"1\" 100)\n");
    fprintf(gp, "set palette defined (%s)\n", paletteDefine.c_str());
    fprintf(gp, "set cbrange[0:%d]\n", maxFailCount);
    fprintf(gp, "set cbtics (%s)\n", cbtics.c_str());
    fprintf(gp, "splot \"%s\" matrix with pm3d\n", intermediateFileName.c_str());
    pclose(gp);
}

int main()
{

    Gnuplot gp;
    gp.readShmFile("data2.txt");
    gp.plot();
}
