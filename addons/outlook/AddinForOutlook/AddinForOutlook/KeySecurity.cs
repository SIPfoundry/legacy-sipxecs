//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
using System;
using System.Security.Cryptography;
using System.IO;
using System.Text;

namespace AddinForOutlook
{
    public class Crypto
    {
        static byte[] _KeySeed = { 0x30, 0x51, 0x41, 0x5A, 0x58, 0x53, 0x57, 0x31};    // 8 * 8 for DES64         

        public static string Encrypt(string source)
        {
            try
            {
                DESCryptoServiceProvider DesProv = new DESCryptoServiceProvider();

                DesProv.Key = _KeySeed;
                DesProv.IV = _KeySeed;

                System.IO.MemoryStream ms = new System.IO.MemoryStream();
                ICryptoTransform encrypto = DesProv.CreateEncryptor();
                CryptoStream cs = new CryptoStream(ms, encrypto, CryptoStreamMode.Write);

                byte[] bytIn = System.Text.ASCIIEncoding.ASCII.GetBytes(source);
                cs.Write(bytIn, 0, bytIn.Length);
                cs.FlushFinalBlock();

                byte[] byteOut = ms.GetBuffer();

                int i = 0;
                for (; i < byteOut.Length; i++)
                {
                    if (byteOut[i] == 0)
                        break;
                }

                cs.Close();
                ms.Close();

                return System.Convert.ToBase64String(byteOut, 0, i);
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, ex.GetType() + ":" + ex.Message);
                return String.Empty;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="Source"></param>
        /// <returns></returns>
        public static string Decrypt(string source)
        {

            try
            {
                DESCryptoServiceProvider DesProv = new DESCryptoServiceProvider();
                DesProv.Key = _KeySeed;
                DesProv.IV = _KeySeed;

                byte[] byteIn = System.Convert.FromBase64String(source);
                System.IO.MemoryStream ms = new System.IO.MemoryStream(byteIn, 0, byteIn.Length);

                ICryptoTransform decrypto = DesProv.CreateDecryptor();
                CryptoStream cs = new CryptoStream(ms, decrypto, CryptoStreamMode.Read);
                System.IO.StreamReader sr = new System.IO.StreamReader(cs);

                String plaintext = sr.ReadToEnd();

                cs.Close();
                sr.Close();

                return plaintext;
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, ex.GetType() + ":" + ex.Message);
                return String.Empty;
            }
        }	
   }
}