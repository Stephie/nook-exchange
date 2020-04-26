module Styles = {
  open Css;
  let emptyProfile =
    style([
      maxWidth(px(768)),
      margin3(~top=zero, ~bottom=px(128), ~h=auto),
      textAlign(center),
    ]);
  let username =
    style([
      fontSize(px(36)),
      textAlign(center),
      marginTop(px(32)),
      marginBottom(px(32)),
    ]);
  let userBody =
    style([
      backgroundColor(hex("ffffffc0")),
      boxSizing(borderBox),
      lineHeight(px(20)),
      margin3(~top=zero, ~bottom=px(48), ~h=auto),
      maxWidth(px(512)),
      padding2(~v=px(16), ~h=px(24)),
      borderRadius(px(8)),
      whiteSpace(`preLine),
      media(
        "(max-width: 512px)",
        [
          borderRadius(zero),
          padding(px(16)),
          marginBottom(zero),
          borderBottom(px(1), solid, Colors.faintGray),
        ],
      ),
    ]);
  let bodyText = style([fontSize(px(18))]);
};

[@react.component]
let make = (~username, ~urlRest, ~url: ReasonReactRouter.url, ~showLogin) => {
  let listStatus =
    switch (urlRest) {
    | [url] => User.urlToItemStatus(url)
    | _ => None
    };
  let (user, setUser) = React.useState(() => None);
  let isMountedRef = React.useRef(true);
  React.useEffect0(() => {
    Some(() => {React.Ref.setCurrent(isMountedRef, false)})
  });
  React.useEffect1(
    () => {
      {
        let%Repromise.JsExn response =
          Fetch.fetchWithInit(
            Constants.apiUrl ++ "/users2/" ++ username,
            Fetch.RequestInit.make(
              ~headers=
                Fetch.HeadersInit.make({
                  "X-Client-Version": Constants.gitCommitRef,
                }),
              (),
            ),
          );
        switch (Fetch.Response.status(response)) {
        | 200 =>
          let%Repromise.JsExn json = Fetch.Response.json(response);
          if (React.Ref.current(isMountedRef)) {
            setUser(_ => Some(User.fromAPI(json)));
          };
          Promise.resolved();
        | _ => Promise.resolved()
        };
      }
      |> ignore;
      None;
    },
    [|username|],
  );
  React.useEffect0(() => {
    switch (url.hash) {
    | "for-trade"
    | "can-craft"
    | "wishlist" =>
      if (urlRest == []) {
        ReasonReactRouter.replace(
          "/"
          ++ Js.Array.joinWith("/", Belt.List.toArray(url.path))
          ++ "/"
          ++ url.hash,
        );
      }
    | _ => ()
    };
    None;
  });
  <div>
    <div className=Styles.username> {React.string(username)} </div>
    {switch (user) {
     | Some(user) =>
       <div>
         {switch (user.profileText) {
          | "" => React.null
          | profileText =>
            <div className=Styles.userBody>
              {Emoji.parseText(profileText)}
            </div>
          }}
         {switch (listStatus) {
          | Some(listStatus) => <UserListBrowser user listStatus url />
          | None =>
            if (user.items->Js.Dict.keys->Js.Array.length > 0) {
              <UserProfileBrowser
                username
                userItems={
                  user.items
                  ->Js.Dict.entries
                  ->Belt.Array.mapU((. (itemKey, item)) =>
                      (User.fromItemKey(~key=itemKey), item)
                    )
                }
                editable=false
              />;
            } else {
              <div className=Styles.emptyProfile>
                <div className=Styles.bodyText>
                  {React.string("I have no lists!")}
                </div>
              </div>;
            }
          }}
       </div>
     | None => React.null
     }}
  </div>;
};