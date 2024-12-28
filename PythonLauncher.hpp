#ifndef PYTHONLAUNCHER_HPP
#define PYTHONLAUNCHER_HPP

#include <QString>
#include <QMessageBox>
#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>

class PythonLauncher : public QObject
{
    Q_OBJECT

public:

    /**
     * @brief Run the Python script. If a virtual environment has been created,
     *        the script will run using that environment's Python.
     * @return The exit code of the Python process.
     */
    int run()
    {
        if (pythonExecutable.isEmpty())
            pythonExecutable = QStringLiteral("python");

        QProcess pythonProcess;

        pythonProcess.setProcessChannelMode(QProcess::MergedChannels);

        QStringList args;

        args << filePath;
        args << arguments;

        pythonProcess.start(pythonExecutable, args);

        if (!pythonProcess.waitForStarted())
        {
            outputString = QStringLiteral("Failed to start the python process.");
            return -1;
        }

        pythonProcess.waitForFinished();

        outputString = pythonProcess.readAll();

        return pythonProcess.exitCode();
    }

    /**
     * @brief Create a virtual environment in the given directory and optionally
     *        install the specified Python modules.
     * @param directory The directory where the new venv will be created.
     * @param modules   A list of modules to install via pip (e.g. {"numpy", "pandas"}).
     *
     * @note Must be called before run().
     */
    void addVirtualEnvironment(const QString& directory, const QList<QString>& modules = {})
    {
        if (directory.isEmpty())
            return;

        {
            QProcess venvProcess;
            QStringList args;

            args << QStringLiteral("-m")
                 << QStringLiteral("venv")
                 << directory;

            venvProcess.start(QStringLiteral("python"), args);

            if (!venvProcess.waitForStarted())
            {
                qWarning() << "Failed to start python -m venv process.";
                return;
            }

            venvProcess.waitForFinished();

            if (venvProcess.exitCode() != 0)
            {
                qWarning() << "Creating virtual environment failed:" << venvProcess.readAll();
                return;
            }
        }

#if defined(Q_OS_WIN)
        pythonExecutable = QDir(directory).filePath("Scripts/python.exe");
#else
        pythonExecutable = QDir(directory).filePath("bin/python");
#endif

        if (!modules.isEmpty())
        {
            QProcess installProcess;
            QStringList installArgs;

            installArgs << QStringLiteral("-m")
                        << QStringLiteral("pip")
                        << QStringLiteral("install");

            installArgs << modules;

            installProcess.start(pythonExecutable, installArgs);

            if (!installProcess.waitForStarted())
            {
                qWarning() << "Failed to start pip install process.";
                return;
            }

            installProcess.waitForFinished();

            if (installProcess.exitCode() != 0)
            {
                qWarning() << "Installing modules failed:" << installProcess.readAll();
                return;
            }
        }
    }

    /**
     * @brief Retrieve the combined standard output and standard error from
     *        the most recent Python run.
     */
    QString getOutput() const
    {
        return outputString;
    }

    /**
     * @brief Create a shared instance of PythonLauncher. This factory method
     *        takes the path to the .py file and the arguments that will be
     *        passed to the script.
     * @param filePath  The Python script to run (absolute or relative path).
     * @param arguments A list of arguments for the Python script.
     *
     * @return A shared pointer to a new PythonLauncher.
     */
    static QSharedPointer<PythonLauncher> create(const QString& filePath, const QList<QString>& arguments)
    {
        QSharedPointer<PythonLauncher> result(new PythonLauncher);

        result->filePath = filePath;
        result->arguments = arguments;

        return result;
    }

private:

    PythonLauncher() = default;

    QString filePath;
    QList<QString> arguments;
    QString outputString;

    QString pythonExecutable;
};


#endif // PYTHONLAUNCHER_HPP
