#include <Wink/client.h>

void Usage(std::string name) {
    Info() << name << '\n';
    Info() << "\tstart [options] <binary> <host>\n";
    Info() << "\tstop [options] <machine>\n";
    Info() << "\tsend [options] <machine> <message>\n";
    Info() << "\tlist [options] <host>\n";
    Info() << "\thelp\n";
}

void Usage() { Usage("Wink"); }

void Help(std::string name, std::string command) {
    if (command == "start") {
        Info() << "Start a new machine.\n\n";
        Info() << "Options;";
        Info() << "\n\t-a\n\t\tThe address to bind to (default " << kLocalhost << ")\n";
        Info() << "\n\t-f\n\t\tFollow the lifecycle of the machine (default false)\n";
        Info() << "Parameters;";
        Info() << "\n\tbinary\n\t\tThe machine binary to start\n";
        Info() << "\n\thost\n\t\tThe host to start the machine on\n";
        Info() << "Examples;";
        Info() << "\n\tstart machine.bin\n\t\tStart a new machine on localhost on any available port\n";
        Info() << "\n\tstart machine.bin :64646\n\t\tStart a new machine on localhost port 64646\n";
        Info() << "\n\tstart machine.bin 123.45.67.89\n\t\tStart a new machine on ip 123.45.67.89 any available port\n";
        Info() << "\n\tstart machine.bin 123.45.67.89:64646\n\t\tStart a new machine on ip 123.45.67.89 port 64646\n";
    } else if (command == "stop") {
        Info() << "Stop an existing machine.\n\n";
        Info() << "Options;";
        // Info() << "\n\t--test\n\t\tTest (default 5)\n";
        Info() << "Parameters;";
        Info() << "\n\tmachine\n\t\tThe machine to stop\n";
        Info() << "Examples;";
        Info() << "\n\tstop 123.45.67.89:64646\n\t\tStop an existing machine on ip 123.45.67.89 port 64646\n";
    } else if (command == "send") {
        Info() << "Sends a message to a machine\n\n";
        Info() << "Options;";
        // Info() << "\n\t--test\n\t\tTest (default 5)\n";
        Info() << "Parameters;";
        Info() << "\n\tmachine\n\t\tThe machine to send to\n";
        Info() << "\n\tmessage\n\t\tThe message to send\n";
        Info() << "Examples;";
        Info() << "\n\tsend :64646 add(2,8)\n\t\tSend a message to machine on localhost port 64646\n";
        Info() << "\n\tsend 123.45.67.89:64646 add(2,8)\n\t\tSend a message to machine on ip 123.45.67.89 port 64646\n";
    } else if (command == "list") {
        Info() << "List machines running on a host\n\n";
        Info() << "Options;";
        // Info() << "\n\t--test\n\t\tTest (default 5)\n";
        Info() << "Parameters;";
        Info() << "\n\thost\n\t\tThe host to list the machines from\n";
        Info() << "Examples;";
        Info() << "\n\tlist\n\t\tLists the machines running on localhost port 42000\n";
        Info() << "\n\tlist :64646\n\t\tLists the machines running on localhost port 64646\n";
        Info() << "\n\tlist 123.45.67.89:64646\n\t\tLists the machines running on ip 123.45.67.89 port 64646\n";
    } else {
        Usage();
    }
}

int main(int argc, char **argv) {
    if (argc <= 0) {
        Usage();
        return 0;
    }
    std::string name(argv[0]);
    if (argc == 1) {
        Usage(name);
        return 0;
    }

    std::string command(argv[1]);

    std::map<std::string, std::string> options;
    std::vector<std::string> parameters;
    for (int i = 2; i < argc; ++i) {
        if (argv[i][0] == '-' && i + 1 < argc) {
            options.insert(std::make_pair(std::string(argv[i]), std::string(argv[i + 1])));
            i++;
        } else {
            parameters.push_back(std::string(argv[i]));
        }
    }

    if (command == "start") {
        Address address(kLocalhost, 0);
        bool follow = false;

        // Parse Options
        for (const auto &[k, v] : options) {
            if (k == "-a") {
                std::stringstream ss(v);
                ss >> address;
            } else if (k == "-f") {
                std::stringstream ss(v);
                std::string f;
                ss >> f;
                std::transform(f.begin(), f.end(), f.begin(), [](uchar c) { return std::tolower(c); });
                follow = (f == "1" || f == "true");
            } else {
                Error() << "Option " << k << ":" << v << " not supported\n" << std::flush;
            }
        }

        Address destination(kLocalhost, 0);
        std::vector<std::string> args;
        auto count = parameters.size();
        if (count == 0) {
            Error() << "Missing <binary> parameter\n" << std::flush;
            return -1;
        }
        std::string binary(parameters.at(0));
        if (count > 1) {
            destination.FromString(parameters.at(1));
            for (uint i = 2; i < count; i++) {
                args.push_back(parameters.at(i));
            }
        }

        UDPSocket socket;
        return StartMachine(socket, address, binary, destination, args, follow);
    } else if (command == "stop") {
        // TODO Parse Options
        for (const auto &[k, v] : options) {
            Error() << "Option " << k << ":" << v << " not supported\n" << std::flush;
        }

        Address address(kLocalhost, 0);
        switch (parameters.size()) {
        case 0:
            Error() << "Missing <machine> parameter\n" << std::flush;
            return -1;
        case 1:
            {
                std::istringstream ss(parameters.at(0));
                ss >> address;
            }
            break;
        default:
            Error() << "Too many parameters\n" << std::flush;
            return -1;
        }

        UDPSocket socket;
        return StopMachine(socket, address);
    } else if (command == "send") {
        // TODO Parse Options
        for (const auto &[k, v] : options) {
            Error() << "Option " << k << ":" << v << " not supported\n" << std::flush;
        }

        Address address(kLocalhost, 0);
        std::string message;
        switch (parameters.size()) {
        case 0:
            Error() << "Missing <machine> parameter\n" << std::flush;
            return -1;
        case 1:
            Error() << "Missing <message> parameter\n" << std::flush;
            return -1;
        case 2:
            {
                const auto s = parameters.at(0);
                std::istringstream ss(s);
                ss >> address;
                message = parameters.at(1);
            }
            break;
        default:
            Error() << "Too many parameters\n" << std::flush;
            return -1;
        }

        UDPSocket socket;
        return SendMessage(socket, address, message);
    } else if (command == "list") {
        // TODO Parse Options
        for (const auto &[k, v] : options) {
            Error() << "Option " << k << ":" << v << " not supported\n" << std::flush;
        }

        Address address(kLocalhost, kServerPort);
        switch (parameters.size()) {
        case 0:
            break;
        case 1:
            {
                const auto s = parameters.at(0);
                std::istringstream ss(s);
                ss >> address;
            }
            break;
        default:
            Error() << "Too many parameters\n" << std::flush;
            return -1;
        }

        UDPSocket socket;
        return ListMachines(socket, address);
    } else if (command == "help") {
        if (argc < 3) {
            Usage();
        } else {
            Help(name, std::string(argv[2]));
        }
    } else {
        Usage(name);
    }
    return 0;
}
