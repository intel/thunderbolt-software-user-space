#include <cinttypes>
#include <tbt/tbt_fwu.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdexcept>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <utility>
#include <string>

#define CK(cmd)                                        \
   do                                                  \
   {                                                   \
      int rc = cmd;                                    \
      if (rc != TBT_OK)                                \
      {                                                \
         fprintf(stderr,                               \
                 "error in %s line %d: 0x%x %s: %s\n", \
                 __FILE__,                             \
                 __LINE__,                             \
                 rc,                                   \
                 tbt_strerror(rc),                     \
                 tbt_lastErrorDetail());               \
         return 1;                                     \
      }                                                \
   } while (0)

class TbtInitter
{
public:
   TbtInitter()
   {
      int rc;
      if ((rc = tbt_fwu_init()) != TBT_OK)
      {
         throw std::runtime_error(tbt_strerror(rc));
      }
   }
   ~TbtInitter() { tbt_fwu_shutdown(); }
};

class AutoFD
{
public:
   AutoFD(int fd) : m_fd(fd) {}
   ~AutoFD()
   {
      if (m_fd < 0)
      {
         return;
      }
      int rc;
      do
      {
         rc = ::close(m_fd);
      } while (rc == -1 && errno == EINTR);
      if (rc == -1)
      {
         fprintf(stderr, "could not close fd: %s\n", strerror(errno));
      }
   }

private:
   int m_fd;
};

std::vector<uint8_t> ReadFile(std::string fn)
{
   std::vector<uint8_t> out;
   int fd = open(fn.c_str(), O_RDONLY);
   if (fd == -1)
   {
      fprintf(stderr, "err reading %s: %s\n", fn.c_str(), strerror(errno));
      return out;
   }
   AutoFD afd(fd);
   char pBuf[4096];
   while (1)
   {
      ssize_t rc;
      do
      {
         rc = read(fd, &pBuf[0], sizeof(pBuf));
      } while (rc == -1 && errno == EINTR);
      if (rc == 0)
      {
         break;
      }
      if (rc == -1)
      {
         fprintf(stderr, "read failed: %s\n", strerror(errno));
         return std::vector<uint8_t>();
      }
      out.insert(out.end(), pBuf, pBuf + rc);
   }

   return out;
}

class TbtControllerListFreer
{
public:
   TbtControllerListFreer(tbt_fwu_Controller** apControllers, int nControllers)
      : m_apControllers(apControllers), m_nControllers(nControllers)
   {
   }
   ~TbtControllerListFreer() { tbt_fwu_freeControllerList(m_apControllers, m_nControllers); }
private:
   tbt_fwu_Controller** m_apControllers;
   int m_nControllers;
};

static std::pair<int, std::string> getControllerID(tbt_fwu_Controller* pCtrl);
static int updateFW(tbt_fwu_Controller* pCtrl, std::string fn, bool bPrompt);

static int validateFWImage(tbt_fwu_Controller* pCtrl, std::string fn);
static int getCurrentNVMVersion(tbt_fwu_Controller* pCtrl);
static int getImageNVMVersion(std::string filename);
static int getModelID(tbt_fwu_Controller* pCtrl);
static int getVendorID(tbt_fwu_Controller* pCtrl);
static int isInSafeMode(tbt_fwu_Controller* pCtrl);

void usage(int argc __attribute__((unused)), char** argv)
{
   printf("usage: %s EnumControllers\n"
          "  or   %s IsInSafeMode <controller>\n"
          "  or   %s ValidateFWImage <controller> <image_filename>\n"
          "  or   %s GetCurrentNVMVersion <controller>\n"
          "  or   %s GetVendorID <controller>\n"
          "  or   %s GetModelID <controller>\n"
          "  or   %s GetImageNVMVersion <image_filename>\n"
          "  or   %s FWUpdate <controller> <image_filename> [--no-prompt]\n"
          "\n"
          "The <controller> argument must be one of the D-Bus controller\n"
          "object names returned by the getControllerList call.  (The\n"
          "controller's own notion of its ID can be retrieved with the\n"
          "getControllerID call.)\n"
          "\n",

          argv[0],
          argv[0],
          argv[0],
          argv[0],
          argv[0],
          argv[0],
          argv[0],
          argv[0]);
}

int main(int argc, char** argv)
{
   struct tbt_fwu_Controller** apControllers;
   size_t nControllers = 0;
   int margc           = argc;
   char** margv        = argv;
   --margc;
   ++margv;

   if (margc == 0)
   {
      usage(argc, argv);
      return 1;
   }
   std::string cmd = margv[0];

   if (cmd == "help" || cmd == "-h" || cmd == "Help")
   {
      usage(argc, argv);
      return 0;
   }

   ++margv;
   --margc;

   if (cmd == "getImageNVMVersion" || cmd == "GetImageNVMVersion")
   {
      if (margc == 0)
      {
         fprintf(stderr, "file name argument required.\n");
         return 1;
      }
      return getImageNVMVersion(margv[0]);
   }

   TbtInitter tbtInit;
   CK(tbt_fwu_getControllerList(&apControllers, &nControllers));
   TbtControllerListFreer freer(apControllers, nControllers);

   if (cmd == "getControllerList" || cmd == "EnumControllers")
   {
      for (size_t i = 0; i < nControllers; ++i)
      {
         auto res = getControllerID(apControllers[i]);
         if (res.first != TBT_OK)
         {
            fprintf(stderr,
                    "could not retrieve ID of controller #%zd (out of %zd): %s\n",
                    i + 1,
                    nControllers,
                    tbt_lastErrorDetail());
            return 1;
         }
         printf("%s\n", res.second.c_str());
      }
      return 0;
   }

   if (margc == 0)
   {
      fprintf(stderr, "controller name argument required.\n");
      return 1;
   }

   tbt_fwu_Controller* pCtrl = nullptr;
   for (size_t i = 0; i < nControllers; ++i)
   {
      auto res = getControllerID(apControllers[i]);
      if (res.first == TBT_OK && res.second == margv[0])
      {
         pCtrl = apControllers[i];
         break;
      }
   }
   if (!pCtrl)
   {
      fprintf(stderr, "no such controller %s or error in retrieving controller ID\n", margv[0]);
      return 1;
   }
   ++margv;
   --margc;

   if (cmd == "updateFW" || cmd == "FWUpdate")
   {
      if (margc == 0)
      {
         fprintf(stderr, "updateFW requires <controller> and <filename> arguments.\n");
         return 1;
      }
      bool bPrompt = true;
      if (margc > 1 && strcmp(margv[1], "--no-prompt") == 0)
      {
         bPrompt = false;
      }
      return updateFW(pCtrl, margv[0], bPrompt);
   }
   else if (cmd == "validateFWImage" || cmd == "ValidateFWImage")
   {
      if (margc == 0)
      {
         fprintf(stderr,
                 "validateFWImage requires <controller> and <filename> "
                 "arguments.\n");
         return 1;
      }
      return validateFWImage(pCtrl, margv[0]);
   }
   else if (cmd == "getCurrentNVMVersion" || cmd == "GetCurrentNVMVersion")
   {
      return getCurrentNVMVersion(pCtrl);
   }
   else if (cmd == "getVendorID" || cmd == "GetVendorID")
   {
      return getVendorID(pCtrl);
   }
   else if (cmd == "getModelID" || cmd == "GetModelID")
   {
      return getModelID(pCtrl);
   }
   else if (cmd == "isInSafeMode" || cmd == "IsInSafeMode")
   {
      return isInSafeMode(pCtrl);
   }
   else
   {
      fprintf(stderr, "invalid command: %s\n", cmd.c_str());
      return 1;
   }
}

static int updateFW(tbt_fwu_Controller* pCtrl, std::string fn, bool bPrompt)
{
   std::vector<uint8_t> fwimg = ReadFile(fn);
   if (fwimg.size() == 0)
   { // read failed or empty file
      return 1;
   }

   int rc;

   if (bPrompt)
   {
      uint32_t cur_maj = 0, cur_min = 0, img_maj = 0, img_min = 0;
      std::string cur_pd, img_pd;
      std::string sControllerID;
      rc                     = tbt_fwu_getImageNVMVersion(&fwimg[0], fwimg.size(), &img_maj, &img_min);
      bool bHasImgNVMVersion = true;
      if (rc != TBT_OK)
      {
         fprintf(stderr,
                 "Warning: could not determine NVM version in FW "
                 "image %s: %s\n",
                 fn.c_str(),
                 tbt_lastErrorDetail());
         bHasImgNVMVersion = false;
      }

      sControllerID = getControllerID(pCtrl).second;

      rc                    = tbt_fwu_Controller_getNVMVersion(pCtrl, &cur_maj, &cur_min);
      bool bHasFWNVMVersion = true;
      if (rc != TBT_OK)
      {
         fprintf(stderr,
                 "Warning: could not determine current NVM version"
                 ": %s\n",
                 tbt_lastErrorDetail());
         bHasFWNVMVersion = false;
      }

      printf("\nUpdate this controller: %s\n"
             "to the image in file:\n%s\n\n",
             sControllerID.c_str(),
             fn.c_str());
      if (!bHasFWNVMVersion)
      {
         printf("current NVM version: ** unknown **\n");
      }
      else
      {
         printf("current NVM version: %" PRIx32 ".%02" PRIx32 "\n", cur_maj, cur_min);
      }
      printf("\n");

      if (!bHasImgNVMVersion)
      {
         printf("image NVM version: ** unknown **\n");
      }
      else
      {
         printf("image NVM version: %" PRIx32 ".%02" PRIx32 "\n", img_maj, img_min);
      }

      char* zLine = nullptr;
      ssize_t glrc;
      size_t nLine = 0;
      bool proceed = false;
      do
      {
         printf("\nWould you like to proceed? [yes/no] ");
         fflush(stdout);
         do
         {
            glrc = getline(&zLine, &nLine, stdin);
         } while (glrc == -1 && errno == EINTR);
         if (glrc == -1)
         {
            fprintf(stderr, "Failed to read from stdin: %s\n", strerror(errno));
            break;
         }
         if (strcmp(zLine, "yes\n") == 0)
         {
            proceed = true;
            break;
         }
         else if (strcmp(zLine, "no\n") == 0)
         {
            proceed = false;
            break;
         }
         else
         {
            printf("Please answer 'yes' or 'no'.\n");
         }
      } while (true);
      free(zLine); // Success or fail.
      if (!proceed)
      {
         return 1;
      }
      printf("\nUpdating...");
      fflush(stdout);
      // fall through
   }

   rc = tbt_fwu_Controller_updateFW(pCtrl, &fwimg[0], fwimg.size());
   if (rc != TBT_OK)
   {
      fprintf(stderr, "FW update failed: 0x%x %s\n", rc, tbt_lastErrorDetail());
   }
   else
   {
      fprintf(stderr, "FW update succeeded.\n");
   }

   return rc;
}

static int getCurrentNVMVersion(tbt_fwu_Controller* pCtrl)
{
   uint32_t major, minor;
   int rc = tbt_fwu_Controller_getNVMVersion(pCtrl, &major, &minor);
   if (rc != TBT_OK)
   {
      fprintf(stderr, "getCurrentNVMVersion failed: 0x%x %s\n", rc, tbt_lastErrorDetail());
   }
   else
   {
      printf("%" PRIx32 ".%02" PRIx32 "\n", major, minor);
   }

   return rc;
}

static int getModelID(tbt_fwu_Controller* pCtrl)
{
   uint16_t id;
   int rc = tbt_fwu_Controller_getModelID(pCtrl, &id);
   if (rc != TBT_OK)
   {
      fprintf(stderr, "getModelID failed: 0x%x %s\n", rc, tbt_lastErrorDetail());
   }
   else
   {
      printf("0x%04" PRIx16 "\n", id);
   }

   return rc;
}

static int getVendorID(tbt_fwu_Controller* pCtrl)
{
   uint16_t id;
   int rc = tbt_fwu_Controller_getVendorID(pCtrl, &id);
   if (rc != TBT_OK)
   {
      fprintf(stderr, "getVendorID failed: 0x%x %s\n", rc, tbt_lastErrorDetail());
   }
   else
   {
      printf("0x%04" PRIx16 "\n", id);
   }

   return rc;
}

static int getImageNVMVersion(std::string filename)
{
   uint32_t major, minor;
   std::vector<uint8_t> image = ReadFile(filename);
   if (image.size() == 0)
   { // read failed or empty file
      return 1;
   }
   int rc = tbt_fwu_getImageNVMVersion(&image[0], image.size(), &major, &minor);
   if (rc != TBT_OK)
   {
      fprintf(stderr, "getImageNVMVersion failed: 0x%x %s\n", rc, tbt_lastErrorDetail());
   }
   else
   {
      printf("%" PRIx32 ".%02" PRIx32 "\n", major, minor);
   }

   return rc;
}

static std::pair<int, std::string> getControllerID(tbt_fwu_Controller* pCtrl)
{
   char zID[64];
   size_t nID = sizeof(zID);
   int rc     = tbt_fwu_Controller_getID(pCtrl, zID, &nID);
   if (rc == TBT_OK)
   {
      return {rc, std::string(zID, zID + nID - 1)};
   }
   return {rc, {}};
}

static int validateFWImage(tbt_fwu_Controller* pCtrl, std::string fn)
{
   std::vector<uint8_t> fwimg = ReadFile(fn);
   if (fwimg.size() == 0)
   { // read failed or empty file
      return 1;
   }

   int rc = tbt_fwu_Controller_validateFWImage(pCtrl, &fwimg[0], fwimg.size());
   if (rc != TBT_OK)
   {
      fprintf(stderr, "Image validation failed: 0x%x %s\n", rc, tbt_lastErrorDetail());
   }
   else
   {
      fprintf(stderr, "Image validation succeeded.\n");
   }

   return rc;
}

static int isInSafeMode(tbt_fwu_Controller* pCtrl)
{
   int bSafeMode;
   int rc = tbt_fwu_Controller_isInSafeMode(pCtrl, &bSafeMode);
   if (rc != TBT_OK)
   {
      fprintf(stderr, "isInSafeMode failed: 0x%x %s\n", rc, tbt_lastErrorDetail());
   }
   else
   {
      printf("%s\n", bSafeMode ? "true" : "false");
   }
   return rc;
}
