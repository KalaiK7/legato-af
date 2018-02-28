//--------------------------------------------------------------------------------------------------
/**
 * @file mkCommon.cpp
 *
 * Copyright (C) Sierra Wireless Inc.
 **/
//--------------------------------------------------------------------------------------------------

#include "mkTools.h"
#include "mkCommon.h"
#include <string.h>
#include <unistd.h>


namespace
{


//----------------------------------------------------------------------------------------------
/**
 * Get the path to a specific build tool.
 *
 * @return  The path to the tool, or an empty string if not specified.
 */
//----------------------------------------------------------------------------------------------
std::string GetToolPath
(
    const std::string& target,          ///< The target device type (e.g., wp85)
    const std::string& toolEnvVarName   ///< Base name of tool path env var (e.g., "CC")
)
//--------------------------------------------------------------------------------------------------
{
    // If the all-caps, target-specific tool path env var (e.g., "WP85_CC") is set, then use that.
    auto allCapsPrefix = target + "_";
    std::transform(allCapsPrefix.begin(), allCapsPrefix.end(), allCapsPrefix.begin(), ::toupper);
    auto path = envVars::Get(allCapsPrefix + toolEnvVarName);
    if (! path.empty())
    {
        return path;
    }

    // Else, if the target-specific tool path env var (e.g., "wp85_CC") is set, then use that.
    path = envVars::Get(target + "_" + toolEnvVarName);
    if (! path.empty())
    {
        return path;
    }

    // Else, if the tool path env var (e.g., "CC") is set, use that.
    path = envVars::Get(toolEnvVarName);
    if (! path.empty())
    {
        return path;
    }

    // Else look for XXXX_TOOLCHAIN_DIR and/or XXXX_TOOLCHAIN_PREFIX environment variables, and
    // if they're set, use those to generate the tool path, assuming the toolchain is the
    // GNU Compiler Collection.
    auto toolChainDir = envVars::Get(allCapsPrefix + "TOOLCHAIN_DIR");   // WP
    if (toolChainDir.empty())
    {
        toolChainDir = envVars::Get(target + "_" + "TOOLCHAIN_DIR");
        if (toolChainDir.empty())
        {
            toolChainDir = envVars::Get("TOOLCHAIN_DIR");
        }
    }
    auto toolChainPrefix = envVars::Get(allCapsPrefix + "TOOLCHAIN_PREFIX");
    if (toolChainPrefix.empty())
    {
        toolChainPrefix = envVars::Get(target + "_" + "TOOLCHAIN_PREFIX");
        if (toolChainPrefix.empty())
        {
            toolChainPrefix = envVars::Get("TOOLCHAIN_PREFIX");
        }
    }

    // toolChainPrefix can be blank, but still valid
    if (toolEnvVarName == "CC")
    {
        path = path::Combine(toolChainDir, toolChainPrefix+"gcc");
    }
    else if (toolEnvVarName == "CXX")
    {
        path = path::Combine(toolChainDir, toolChainPrefix+"g++");
    }
    else
    {
        // By default, convert the tool env var name to all lowercase and use that as the
        // tool executable name.
        auto toolName = toolEnvVarName;
        std::transform(toolName.begin(), toolName.end(), toolName.begin(), ::tolower);
        path = path::Combine(toolChainDir, toolChainPrefix+toolName);
    }

    return path;
}


//----------------------------------------------------------------------------------------------
/**
 * Get the sysroot path to use when linking for a given compiler.
 *
 * @return  The path to the sysroot base directory, or an empty string if not specified.
 *
 * @throws  mk::Exception_t on error.
 */
//----------------------------------------------------------------------------------------------
std::string GetSysRootPath
(
    const std::string& target,          ///< The target device type (e.g., wp85)
    const std::string& cCompilerPath    ///< Path to the C compiler
)
//--------------------------------------------------------------------------------------------------
{
    // If LEGATO_SYSROOT is set, use that.
    auto sysRoot = envVars::Get("LEGATO_SYSROOT");
    if (! sysRoot.empty())
    {
        return sysRoot;
    }

    // Else, if the target-specific XXXX_SYSROOT is set, then use that.
    std::string targetPrefix = target + "_";
    std::transform(targetPrefix.begin(), targetPrefix.end(), targetPrefix.begin(), ::toupper);
    sysRoot = envVars::Get(targetPrefix + "SYSROOT");
    if (! sysRoot.empty())
    {
        return sysRoot;
    }

    // Else, if the compiler is gcc, ask gcc what sysroot it uses by default.
    if (path::HasSuffix(cCompilerPath, "gcc"))
    {
        std::string commandLine = cCompilerPath + " --print-sysroot";

        FILE* output = popen(commandLine.c_str(), "r");

        if (output == NULL)
        {
            throw mk::Exception_t(
                mk::format(LE_I18N("Could not exec '%s' to get sysroot path."), commandLine)
            );
        }

        char buffer[1024] = { 0 };
        static const size_t bufferSize = sizeof(buffer);

        if (fgets(buffer, bufferSize, output) != buffer)
        {
            std::cerr <<
                mk::format(LE_I18N("** WARNING: Failed to receive sysroot path from compiler "
                                   "'%s' (errno: %s)."), commandLine, strerror(errno))
                      << std::endl;
            buffer[0] = '\0';
        }
        else
        {
            // Remove any trailing newline character.
            size_t len = strlen(buffer);
            if (buffer[len - 1] == '\n')
            {
                buffer[len - 1] = '\0';
            }
        }

        // Yocto >= 1.8 returns '/not/exist' as a sysroot path
        if (buffer == std::string("/not/exist"))
        {
            std::cerr << mk::format(LE_I18N("** WARNING: Invalid sysroot returned from compiler"
                                            " '%s' (returned '%s')."), commandLine, buffer)
                      << std::endl;
            buffer[0] = '\0';
        }

        // Close the connection and collect the exit code from the compiler.
        int result;
        do
        {
            result = pclose(output);

        } while ((result == -1) && (errno == EINTR));

        if (result == -1)
        {
            throw mk::Exception_t(
                mk::format(LE_I18N("Failed to receive the sysroot path from the compiler '%s'. "
                                   "pclose() errno = %s"), commandLine, strerror(errno))
            );
        }
        else if (!WIFEXITED(result))
        {
            throw mk::Exception_t(
                mk::format(LE_I18N("Failed to receive the sysroot path from the compiler '%s'. "
                                   "Compiler was interrupted by something."),
                           commandLine)
            );
        }
        else if (WEXITSTATUS(result) != EXIT_SUCCESS)
        {
            throw mk::Exception_t(
                mk::format(LE_I18N("Failed to receive the sysroot path from the compiler '%s'. "
                                   "Compiler exited with code %d"),
                           commandLine, WEXITSTATUS(result))
            );
        }

        return buffer;
    }

    return sysRoot;
}


} // anonymous namespace


namespace cli
{


//--------------------------------------------------------------------------------------------------
/**
 * Figure out what compiler, linker, etc. to use based on the target device type and store that
 * info in the @c buildParams object.
 *
 * If a given tool is not specified through the means documented in
 * @ref buildToolsmk_ToolChainConfig, then the corresponding entry in @c buildParams will be
 * left empty.
 */
//--------------------------------------------------------------------------------------------------
void FindToolChain
(
    mk::BuildParams_t& buildParams  ///< [in,out] target member must be set before call
)
//--------------------------------------------------------------------------------------------------
{
    buildParams.cCompilerPath = GetToolPath(buildParams.target, "CC");
    buildParams.cxxCompilerPath = GetToolPath(buildParams.target, "CXX");
    buildParams.sysrootPath = GetSysRootPath(buildParams.target, buildParams.cCompilerPath);
    buildParams.linkerPath = GetToolPath(buildParams.target, "LD");
    buildParams.archiverPath = GetToolPath(buildParams.target, "AR");
    buildParams.assemblerPath = GetToolPath(buildParams.target, "AS");
    buildParams.stripPath = GetToolPath(buildParams.target, "STRIP");
    buildParams.objcopyPath = GetToolPath(buildParams.target, "OBJCOPY");
    buildParams.readelfPath = GetToolPath(buildParams.target, "READELF");

    if (buildParams.beVerbose)
    {
        std::cout << "C compiler = " << buildParams.cCompilerPath << std::endl;
        std::cout << "C++ compiler =" << buildParams.cxxCompilerPath << std::endl;
        std::cout << "Compiler sysroot = " << buildParams.sysrootPath << std::endl;
        std::cout << "Linker = " << buildParams.linkerPath << std::endl;
        std::cout << "Static lib archiver = " << buildParams.archiverPath << std::endl;
        std::cout << "Assembler = " << buildParams.assemblerPath << std::endl;
        std::cout << "Debug symbol stripper = " << buildParams.stripPath << std::endl;
        std::cout << "Object file copier/translator = " << buildParams.objcopyPath << std::endl;
        std::cout << "ELF file info extractor = " << buildParams.readelfPath << std::endl;
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Checks the info in @c buildParams object for IMA signing. If nothing specified in @c buildParams
 * object then check environment variables for IMA signing and update @c buildParams object
 * accordingly.
 */
//--------------------------------------------------------------------------------------------------
void CheckForIMASigning
(
    mk::BuildParams_t& buildParams
)
{
    // No ima sign flag is provided in command line, so check environment variable ENABLE_IMA
    if (!buildParams.signPkg)
    {
        std::string imaEnable = envVars::Get("ENABLE_IMA");
        if (imaEnable.compare("1") == 0)
        {
            buildParams.signPkg = true;
        }
    }

    if (buildParams.signPkg)
    {
        // Get key value from environment variable if no path is specified
        if (buildParams.privKey.empty() && buildParams.pubCert.empty())
        {
            buildParams.privKey = envVars::GetRequired("IMA_PRIVATE_KEY");
            buildParams.pubCert = envVars::GetRequired("IMA_PUBLIC_CERT");
        }

        // Now checks whether private key exists or not. No need to check empty value, if privKey
        // string is empty FileExists() function should return false.
        if (!file::FileExists(buildParams.privKey))
        {
            throw mk::Exception_t(mk::format(LE_I18N("Bad private key location '%s'. Provide path "
                                             "via environment variable IMA_PRIVATE_KEY or -K flag"),
                                             buildParams.privKey));
        }

        // Now checks whether public certificate exists or not
        if (!file::FileExists(buildParams.pubCert))
        {
            throw mk::Exception_t(mk::format(LE_I18N("Bad public certificate location '%s'. Provide"
                                             " path via environment variable IMA_PUBLIC_CERT or -P"
                                             " flag"),
                                             buildParams.pubCert));
        }
    }
    else
    {
        if ((false == buildParams.privKey.empty()) || (false == buildParams.pubCert.empty()))
        {
            throw mk::Exception_t(LE_I18N("Wrong option. Sign(-S) option or environment variable "
                                  "ENABLE_IMA must be set to sign the package."));
        }
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Run the Ninja build tool.  Executes the build.ninja script in the root of the working directory
 * tree, if it exists.
 *
 * If the build.ninja file exists, this function will never return.  If the build.ninja file doesn't
 * exist, then this function WILL (quietly) return.
 *
 * @throw mk::Exception if the build.ninja file exists but ninja can't be run.
 */
//--------------------------------------------------------------------------------------------------
void RunNinja
(
    const mk::BuildParams_t& buildParams
)
//--------------------------------------------------------------------------------------------------
{
    auto ninjaFilePath = path::Combine(buildParams.workingDir, "build.ninja");

    if (file::FileExists(ninjaFilePath))
    {
        if (buildParams.beVerbose)
        {
            std::cout << LE_I18N("Executing ninja build system...") << std::endl;
            std::cout << mk::format(LE_I18N("$ ninja -v -d explain -f %s"), ninjaFilePath)
                      << std::endl;

            (void)execlp("ninja",
                         "ninja",
                         "-v",
                         "-d", "explain",
                         "-f", ninjaFilePath.c_str(),
                         (char*)NULL);

            // REMINDER: If you change the list of arguments passed to ninja, don't forget to
            //           update the std::cout message above that to match your changes.
        }
        else
        {
            (void)execlp("ninja",
                         "ninja",
                         "-f", ninjaFilePath.c_str(),
                         (char*)NULL);
        }

        int errCode = errno;

        throw mk::Exception_t(
            mk::format(LE_I18N("Failed to execute ninja (%s)."), strerror(errCode))
        );
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Generate code for a given component.
 */
//--------------------------------------------------------------------------------------------------
void GenerateCode
(
    model::Component_t* componentPtr,
    const mk::BuildParams_t& buildParams
)
//--------------------------------------------------------------------------------------------------
{
    // Create a working directory to build the component in.
    file::MakeDir(path::Combine(buildParams.workingDir, componentPtr->workingDir));

    // Generate a custom "interfaces.h" file for this component.
    code::GenerateInterfacesHeader(componentPtr, buildParams);

    // Generate a custom "_componentMain.c" file for this component.
    code::GenerateComponentMainFile(componentPtr, buildParams);
}


//--------------------------------------------------------------------------------------------------
/**
 * Generate code for all the components in a given set.
 */
//--------------------------------------------------------------------------------------------------
void GenerateCode
(
    const std::set<model::Component_t*>& components,  ///< Set of components to generate code for.
    const mk::BuildParams_t& buildParams
)
//--------------------------------------------------------------------------------------------------
{
    for (auto componentPtr : components)
    {
        GenerateCode(componentPtr, buildParams);
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Generate code for all the components in a given map.
 */
//--------------------------------------------------------------------------------------------------
void GenerateCode
(
    const std::map<std::string, model::Component_t*>& components,
    const mk::BuildParams_t& buildParams
)
//--------------------------------------------------------------------------------------------------
{
    for (auto& mapEntry : components)
    {
        GenerateCode(mapEntry.second, buildParams);
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Generate code specific to an individual app (excluding code for the components).
 */
//--------------------------------------------------------------------------------------------------
void GenerateCode
(
    model::App_t* appPtr,
    const mk::BuildParams_t& buildParams
)
//--------------------------------------------------------------------------------------------------
{
    // Create the working directory, if it doesn't already exist.
    file::MakeDir(path::Combine(buildParams.workingDir, appPtr->workingDir));

    // Generate the configuration data file.
    config::Generate(appPtr, buildParams);

    // For each executable in the application,
    for (auto exePtr : appPtr->executables)
    {
        // Generate _main.c.
        code::GenerateExeMain(exePtr.second, buildParams);
    }
}



} // namespace cli
