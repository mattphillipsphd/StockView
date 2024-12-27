#ifndef PYTHONLAUNCHER_HPP
#define PYTHONLAUNCHER_HPP

#include <QString>
#include <QMessageBox>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>
#define popen _popen
#define pclose _pclose
#define WEXITSTATUS(status) ((status) & 0xFF)
#else
#include <unistd.h>
#endif

class PythonLauncher
{

public:

    PythonLauncher(const PythonLauncher&) = delete;
    PythonLauncher(PythonLauncher&&) = delete;
    PythonLauncher& operator=(const PythonLauncher&) = delete;
    PythonLauncher& operator=(PythonLauncher&&) = delete;

    static int Launch(const QString& filePath, const QList<QString>& arguments, QString& outputString)
    {
        QString command = "python3 ";
        command += filePath;

        for (QString argument : arguments)
            command += " \"" + EscapeArgument(argument) + "\"";

        try
        {
            QString output = ExecuteCommand(command);

            outputString = output;
        }
        catch (const std::runtime_error& e)
        {
            QMessageBox::information(nullptr, "Error!", e.what());
            return 1;
        }

        return 0;
    }

private:

    static QString EscapeArgument(const QString& argument)
    {
        QString result;

        for (QChar c : argument)
        {
            if (c == '\"')
                result += "\\\"";
            else
                result += c;
        }

        return result;
    }

    static QString ExecuteCommand(const QString& command)
    {
        QString result;
        char buffer[128];

        FILE* pipe = popen(command.toStdString().c_str(), "r");

        if (!pipe)
            QMessageBox::information(nullptr, "Error!", "Failed to open pipe for command: " + command);

        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            result += buffer;

        int status = pclose(pipe);

        if (status == -1)
            QMessageBox::information(nullptr, "Error!", "Failed to close pipe");

        else if (WEXITSTATUS(status) != 0)
            QMessageBox::information(nullptr, "Error!", QString("Python script exited with status: ") + (QChar)WEXITSTATUS(status));

        return result;
    }

    PythonLauncher() = default;

};

#endif // PYTHONLAUNCHER_HPP
