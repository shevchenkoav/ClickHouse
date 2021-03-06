#include <iostream>
#include <iomanip>

#include <common/DateLUT.h>

#include <Poco/ConsoleChannel.h>

#include <IO/ReadBufferFromIStream.h>
#include <IO/WriteBufferFromOStream.h>

#include <Storages/StorageLog.h>
#include <Storages/System/attachSystemTables.h>

#include <Interpreters/Context.h>
#include <Interpreters/loadMetadata.h>
#include <Interpreters/executeQuery.h>
#include <Databases/IDatabase.h>
#include <Databases/DatabaseOrdinary.h>


using namespace DB;

int main(int argc, char ** argv)
try
{
    Poco::AutoPtr<Poco::ConsoleChannel> channel = new Poco::ConsoleChannel(std::cerr);
    Logger::root().setChannel(channel);
    Logger::root().setLevel("trace");

    /// Pre-initialize the `DateLUT` so that the first initialization does not affect the measured execution speed.
    DateLUT::instance();

    Context context = Context::createGlobal();

    context.setPath("./");

    loadMetadata(context);

    DatabasePtr system = std::make_shared<DatabaseOrdinary>("system", "./metadata/system/");
    context.addDatabase("system", system);
    system->loadTables(context, nullptr, false);
    attachSystemTablesLocal(*context.getDatabase("system"));
    context.setCurrentDatabase("default");

    ReadBufferFromIStream in(std::cin);
    WriteBufferFromOStream out(std::cout);

    executeQuery(in, out, /* allow_into_outfile = */ false, context, {});

    return 0;
}
catch (const Exception & e)
{
    std::cerr << e.what() << ", " << e.displayText() << std::endl
        << std::endl
        << "Stack trace:" << std::endl
        << e.getStackTrace().toString();
    return 1;
}
