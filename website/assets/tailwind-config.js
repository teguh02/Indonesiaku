/* ============================================================
   Indonesiaku — token desain Tailwind (Play CDN)
   Dimuat SETELAH cdn.tailwindcss.com dan SEBELUM konten dirender.
   Arah desain: editorial-teknis, kertas hangat + tinta, aksen merah.
   ============================================================ */
tailwind.config = {
  darkMode: "class",
  theme: {
    extend: {
      colors: {
        // Merah Indonesia sebagai aksen utama (bukan seluruh permukaan)
        brand: {
          50: "#fdf2f3",
          100: "#fbe3e5",
          200: "#f6c6ca",
          300: "#ee9ca3",
          400: "#e26672",
          500: "#d03a49",
          600: "#b4232f",
          700: "#961c26",
          800: "#7c1a22",
          900: "#66181f",
        },
        // Permukaan terang: kertas hangat, bukan putih dingin
        paper: { DEFAULT: "#faf7f2", 2: "#f3eee5", 3: "#e8dfd1" },
        ink: { DEFAULT: "#12100e", 2: "#3a3530", 3: "#6b6560", 4: "#98918a" },
        // Permukaan gelap: hampir hitam kebiruan, kontras tinggi
        night: { DEFAULT: "#0b0c10", 2: "#12141a", 3: "#1a1d26", 4: "#252a36" },
        gold: "#c98a1e",
      },
      fontFamily: {
        sans: ['"Plus Jakarta Sans"', "system-ui", "-apple-system", "Segoe UI", "sans-serif"],
        mono: ['"JetBrains Mono"', "ui-monospace", "SFMono-Regular", "Consolas", "monospace"],
      },
      fontSize: {
        "7xl": ["4.25rem", { lineHeight: "1.03", letterSpacing: "-0.03em" }],
        "6xl": ["3.5rem", { lineHeight: "1.06", letterSpacing: "-0.025em" }],
        "5xl": ["2.75rem", { lineHeight: "1.1", letterSpacing: "-0.02em" }],
        "4xl": ["2.15rem", { lineHeight: "1.15", letterSpacing: "-0.02em" }],
      },
      borderRadius: { xl2: "1.15rem", xl3: "1.6rem" },
      boxShadow: {
        card: "0 1px 2px rgba(18,16,14,0.04), 0 10px 30px -18px rgba(18,16,14,0.35)",
        lift: "0 2px 4px rgba(18,16,14,0.05), 0 24px 48px -24px rgba(18,16,14,0.45)",
        inset: "inset 0 1px 0 rgba(255,255,255,0.06)",
        ring: "0 0 0 1px rgba(18,16,14,0.06)",
      },
      maxWidth: { prose2: "46rem", wrap: "76rem" },
      transitionTimingFunction: { material: "cubic-bezier(0.2, 0, 0, 1)" },
      keyframes: {
        rise: {
          from: { opacity: "0", transform: "translateY(14px)" },
          to: { opacity: "1", transform: "translateY(0)" },
        },
        fade: { from: { opacity: "0" }, to: { opacity: "1" } },
        blink: { "0%,49%": { opacity: "1" }, "50%,100%": { opacity: "0" } },
      },
      animation: {
        rise: "rise 0.7s cubic-bezier(0.2,0,0,1) both",
        fade: "fade 0.6s ease-out both",
        blink: "blink 1.1s steps(1) infinite",
      },
      backgroundImage: {
        dots: "radial-gradient(rgba(18,16,14,0.14) 1px, transparent 1px)",
        "dots-dark": "radial-gradient(rgba(250,247,242,0.16) 1px, transparent 1px)",
      },
      backgroundSize: { dotgrid: "22px 22px" },
    },
  },
};
