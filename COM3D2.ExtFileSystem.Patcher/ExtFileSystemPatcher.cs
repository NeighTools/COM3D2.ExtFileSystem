using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using Mono.Cecil;
using Mono.Cecil.Cil;

namespace COM3D2.ARCXLoader.Patcher
{
    public class ExtLoaderPatcher
    {
        public static readonly string[] TargetAssemblyNames = {"Assembly-CSharp.dll"};

        public static void Patch(AssemblyDefinition ass)
        {
            var sceneStart = ass.MainModule.GetType("GameUty");
            var start = sceneStart.Methods.First(m => m.Name == ".cctor");

            var il = start.Body.GetILProcessor();
            var ins = start.Body.Instructions.First();

            il.InsertBefore(ins,
                            il.Create(OpCodes.Call,
                                      ass.MainModule.Import(typeof(ExtFileSystemPatcher).GetMethod("Run",
                                                                                         BindingFlags.Public | BindingFlags.Static))));
        }
    }

    public class ExtFileSystemPatcher
    {
        private delegate void InstallDelegate([MarshalAs(UnmanagedType.LPWStr)] string proxyPath);
        private static InstallDelegate installArcX;
        const string PROXYPATH = "Mod_ext";

        public static void Run()
        {
            Console.WriteLine("[ExtFileSystem] Loader started");

            string modExtPath = Path.GetFullPath(PROXYPATH);

            if (!Directory.Exists(modExtPath))
            {
                Console.WriteLine($"Mod path {modExtPath} does not exist! Skipping loading!");
                Directory.CreateDirectory(modExtPath);
            }

            string nativeDll = Path.GetFullPath(Path.Combine(Path.GetDirectoryName(typeof(ExtFileSystemPatcher).Assembly.Location), "ExtFileSystem.dll"));

            var dll = LoadLibrary(nativeDll);

            if (dll == IntPtr.Zero)
            {
                Console.WriteLine($"[ExtFileSystem] ERROR: Failed to find {nativeDll}");
                return;
            }

            var installProc = GetProcAddress(dll, "Install");

            if (installProc == IntPtr.Zero)
            {
                Console.WriteLine($"[ExtFileSystem] ERROR: Invalid ExtFileSystem.dll");
                return;
            }

            installArcX = (InstallDelegate) Marshal.GetDelegateForFunctionPointer(installProc, typeof(InstallDelegate));

            Console.WriteLine("[ExtFileSystem] Installing filesystem");
            installArcX(modExtPath);
        }

        [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
        private static extern IntPtr LoadLibrary(string path);

        [DllImport("kernel32.dll")]
        private static extern IntPtr GetProcAddress(IntPtr module, [MarshalAs(UnmanagedType.LPStr)] string proc);
    }
}