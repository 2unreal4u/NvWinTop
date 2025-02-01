# Combine multiple .ico files into one
$iconutil = @"
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

public class IconUtil {
    public static void CombineIcons(string[] sourceFiles, string outputFile) {
        using (FileStream outStream = new FileStream(outputFile, FileMode.Create)) {
            // Write ICO header
            BinaryWriter writer = new BinaryWriter(outStream);
            writer.Write((short)0);    // Reserved
            writer.Write((short)1);    // Type: ICO
            writer.Write((short)sourceFiles.Length);  // Number of images

            // Calculate offset for image data
            int offset = 6 + (16 * sourceFiles.Length);
            byte[][] imageData = new byte[sourceFiles.Length][];
            
            // Write directory entries
            for (int i = 0; i < sourceFiles.Length; i++) {
                using (FileStream fs = new FileStream(sourceFiles[i], FileMode.Open)) {
                    fs.Seek(0, SeekOrigin.Begin);
                    
                    // Skip ICO header
                    fs.Seek(6, SeekOrigin.Current);
                    
                    // Read first directory entry
                    byte[] dirEntry = new byte[16];
                    fs.Read(dirEntry, 0, 16);
                    
                    // Update offset in directory entry
                    BitConverter.GetBytes(offset).CopyTo(dirEntry, 12);
                    
                    // Write directory entry
                    outStream.Write(dirEntry, 0, 16);
                    
                    // Read image data
                    int size = BitConverter.ToInt32(dirEntry, 8);
                    imageData[i] = new byte[size];
                    
                    // Skip to image data
                    fs.Seek(BitConverter.ToInt32(dirEntry, 12), SeekOrigin.Begin);
                    fs.Read(imageData[i], 0, size);
                    
                    offset += size;
                }
            }
            
            // Write image data
            foreach (byte[] data in imageData) {
                outStream.Write(data, 0, data.Length);
            }
        }
    }
}
"@

Add-Type -TypeDefinition $iconutil -ReferencedAssemblies System.Drawing

$sourceFiles = @(
    "16x16.ico",
    "32x32.ico",
    "48x48.ico",
    "256x256.ico"
)

[IconUtil]::CombineIcons($sourceFiles, "nvwintop.ico")
