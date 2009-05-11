﻿using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;
using SharpSvn.Tests.Commands;
using System.IO;
using NUnit.Framework.SyntaxHelpers;

namespace SharpSvn.Tests.AdminCommands
{
    [TestFixture]
    public class DumpTest : TestBase
    {

        [Test]
        public void DumpDb()
        {
            string repos = GetTempDir();

            using (SvnRepositoryClient cl = new SvnRepositoryClient())
            {
                SvnCreateRepositoryArgs ra = new SvnCreateRepositoryArgs();
                ra.RepositoryType = SvnRepositoryFileSystem.BerkeleyDB;
                ra.RepositoryCompatibility = SvnRepositoryCompatibility.Subversion10;

                cl.CreateRepository(repos, ra);

                string file = GetTempFile();
                using(FileStream s = File.Create(file))
                {
                    cl.DumpRepository(repos, s);
                }

                Assert.That(new FileInfo(file).Length, Is.GreaterThan(12));
            }
        }
    }
}